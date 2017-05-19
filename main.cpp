#include <iostream>
#include <microhttpd.h>
#include <thread>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include "contentmanager.h"

struct CMSPair {
    ContentManager* m_cms;
    MHD_Daemon* m_daemon;
};

int AccessHandlerCallback(void* cls, MHD_Connection* conn, char const* url, char const* method, char const* version, char const* uploadData, size_t* uploadDataSize, void**)
{
    CMSPair* cms_pair = static_cast<CMSPair*>(cls);
    if(!cms_pair)
        return MHD_NO;
    
    bool rv = cms_pair->m_cms->HandleRequest(cms_pair->m_daemon, conn, url, method, version, uploadData, *uploadDataSize);
    *uploadDataSize = 0;
    return rv ? MHD_YES : MHD_NO;
}

struct ProgramOptions {
    enum ConnectionType {
        CT_IPV4 = 0,
        CT_IPV6,
        CT_DUAL_STACK
    };
    
    inline ProgramOptions()
        : m_connType(CT_DUAL_STACK),
          m_port(8080),
          m_threadCount(16),
          m_connectionTimeout(0),
          m_globalConnectionLimit(0),
          m_userConnectionLimit(0),
          m_pedantic(false),
          m_luaThreads(8),
          m_luaFallbackBehavior(LFB_FIND_LUA_STATE)
    {}
    
    ConnectionType m_connType;
    std::uint16_t m_port;
    
    std::uint16_t m_threadCount;
    std::uint16_t m_connectionTimeout;
    std::uint16_t m_globalConnectionLimit;
    std::uint16_t m_userConnectionLimit;
    bool m_pedantic;
    
    std::uint16_t m_luaThreads;
    LuaFallbackBehavior m_luaFallbackBehavior;
};

std::string RandString(std::size_t len)
{
    std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<std::uint8_t> dist;
    
    auto RandChar = [&]() -> char {
        return static_cast<char>(dist(rng));
    };
    
    std::string str(len,0);
    std::generate_n(str.begin(), len, RandChar);
    return str;
}

struct RequiredFeature {
    MHD_FEATURE m_feature;
    std::string m_featureName;
};

static std::vector<RequiredFeature> const g_requiredFeatures = {
    {MHD_FEATURE_BASIC_AUTH, "Basic Authentication"},
    {MHD_FEATURE_DIGEST_AUTH, "Digest Authentication"},
    {MHD_FEATURE_POSTPROCESSOR, "POST Processor"}
};

int main(int, char**)
{
    std::cout << "SGH LuaHTTPD v" << VERSION_MAJOR << '.' << VERSION_MINOR << '.' << VERSION_PATCH << std::endl;
    
    ProgramOptions options;
    options.m_port = 8088;
    
    for(auto it = g_requiredFeatures.begin(); it != g_requiredFeatures.end(); ++it)
    {
        if(!MHD_is_feature_supported(it->m_feature))
        {
            std::cerr << "Error: Your system doesn't support " << it->m_featureName << std::endl;
            return 1;
        }
    }
    if(options.m_connType == ProgramOptions::CT_IPV6)
    {
        if(!MHD_is_feature_supported(MHD_FEATURE_IPv6_ONLY))
        {
            std::cout << "Error: Your system doesn't support IPv6-only sockets." << std::endl;
            std::cout << "Trying Dual-Stack mode..." << std::endl;
            options.m_connType = ProgramOptions::CT_DUAL_STACK;
        }
    }
    if(options.m_connType == ProgramOptions::CT_DUAL_STACK)
    {
        if(!MHD_is_feature_supported(MHD_FEATURE_IPv6))
        {
            std::cout << "Error: Your system doesn't support Dual-Stack mode." << std::endl;
            std::cout << "Trying IPv4 mode..." << std::endl;
            options.m_connType = ProgramOptions::CT_IPV4;
        }
    }
    
    std::cout << "Loading Content Manager..." << std::endl;
    ContentManager cms{options.m_luaThreads, options.m_luaFallbackBehavior};
    CMSPair cms_data;
    cms_data.m_cms = &cms;
    cms_data.m_daemon = nullptr;
    std::cout << "Content Manager loaded." << std::endl;
    
    std::string seed = RandString(64);
    
    
    {
        std::vector<MHD_OptionItem> dops;
        std::uint32_t flags = 0;
        
        switch(options.m_connType)
        {
        default:
        case ProgramOptions::CT_IPV4:
            break;
        case ProgramOptions::CT_IPV6:
            flags = MHD_USE_IPv6;
            break;
        case ProgramOptions::CT_DUAL_STACK:
            flags = MHD_USE_DUAL_STACK;
            break;
        }
        if(options.m_pedantic)
            flags |= MHD_USE_PEDANTIC_CHECKS;
        
        flags |= MHD_USE_SELECT_INTERNALLY;
        
        if(options.m_globalConnectionLimit != 0)
            dops.push_back({MHD_OPTION_CONNECTION_LIMIT, options.m_globalConnectionLimit, nullptr});
        if(options.m_connectionTimeout != 0)
            dops.push_back({MHD_OPTION_CONNECTION_TIMEOUT, options.m_connectionTimeout, nullptr});
        if(options.m_userConnectionLimit != 0)
            dops.push_back({MHD_OPTION_PER_IP_CONNECTION_LIMIT, options.m_userConnectionLimit, nullptr});
        if(options.m_threadCount < 2)
            options.m_threadCount = 2;
        dops.push_back({MHD_OPTION_THREAD_POOL_SIZE, options.m_threadCount, nullptr});
        dops.push_back({MHD_OPTION_NONCE_NC_SIZE, 1000, nullptr});
        dops.push_back({MHD_OPTION_DIGEST_AUTH_RANDOM, static_cast<std::uint32_t>(seed.size()), reinterpret_cast<void*>(&seed[0])});
        dops.push_back({MHD_OPTION_END, 0, nullptr});
        
        cms_data.m_daemon = MHD_start_daemon(flags, options.m_port,
                                  nullptr, nullptr, AccessHandlerCallback, static_cast<void*>(&cms_data),
                                  MHD_OPTION_ARRAY, dops.data(),
                                  MHD_OPTION_END);
    }
    
    if(!cms_data.m_daemon)
    {
        std::cerr << "Error: The server couldn't start." << std::endl;
        return 1;
    }
    std::cout << "Server running" << std::endl;
    
    while(cms.Alive())
    {
        std::this_thread::yield();
    }
    MHD_stop_daemon(cms_data.m_daemon);
    std::cout << "Shutdown successful." << std::endl;
    
    return 0;
}

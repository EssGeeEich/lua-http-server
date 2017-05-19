#include "requestmanager.h"

bool RequestManager::ConnectionInfoT::HTTPS()
{
    return MHD_get_connection_info(Connection(), MHD_CONNECTION_INFO_CIPHER_ALGO) != nullptr;
}

bool RequestManager::ConnectionInfoT::IPV4()
{
    return MHD_get_connection_info(Connection(), MHD_CONNECTION_INFO_CLIENT_ADDRESS)
            ->client_addr->sa_family == AF_INET;
}

bool RequestManager::ConnectionInfoT::IPV6()
{
    return MHD_get_connection_info(Connection(), MHD_CONNECTION_INFO_CLIENT_ADDRESS)
            ->client_addr->sa_family == AF_INET6;
}

#include <sstream>
static std::string boost_inet_ipv4(sockaddr_in* sa)
{
    {
        char buffer[INET_ADDRSTRLEN];
        int err= getnameinfo(reinterpret_cast<sockaddr*>(sa), sizeof(*sa), buffer, sizeof(buffer), nullptr, 0, NI_NUMERICHOST);
        if(!err)
            return buffer;
    }
    
    std::uint32_t addr = ntohl(sa->sin_addr.S_un.S_addr);
    std::uint8_t const* pAddr = reinterpret_cast<std::uint8_t const*>(&addr);
    
    std::ostringstream oss;
    for(std::size_t i = 0; i < 4; ++i)
    {
        if(i)
            oss << '.';
        // _WIN32
        oss << pAddr[i];
    }
    return oss.str();
}


static std::string boost_inet_ipv6(sockaddr_in6* sa)
{
    {
        char buffer[INET6_ADDRSTRLEN];
        int err = getnameinfo(reinterpret_cast<sockaddr*>(sa), sizeof(*sa), buffer, sizeof(buffer), nullptr, 0, NI_NUMERICHOST);
        if(!err)
            return buffer;
    }
    
    std::ostringstream oss;
    for(std::size_t i = 0; i < 8; ++i)
    {
        if(i)
            oss << ':';
        // _WIN32
        oss << std::hex << sa->sin6_addr.u.Word;
    }
    return oss.str();
}

static std::string boost_inet_ipv6_v4(sockaddr_in* sa)
{
    return std::string("::") + boost_inet_ipv4(sa);
}

RequestManager::string RequestManager::ConnectionInfoT::GetIP()
{
    sockaddr* res = MHD_get_connection_info(Connection(), MHD_CONNECTION_INFO_CLIENT_ADDRESS)
            ->client_addr;
    if(!res)
        return RequestManager::string();
    if(res->sa_family == AF_INET)
    {
        sockaddr_in* addr4 = reinterpret_cast<sockaddr_in*>(res);
        return boost_inet_ipv4(addr4);
    }
    else if(res->sa_family == AF_INET6)
    {
        sockaddr_in6* addr6 = reinterpret_cast<sockaddr_in6*>(res);
        return boost_inet_ipv6(addr6);
    }
    return RequestManager::string();
}

RequestManager::string RequestManager::ConnectionInfoT::GetIPV6()
{
    sockaddr* res = MHD_get_connection_info(Connection(), MHD_CONNECTION_INFO_CLIENT_ADDRESS)
            ->client_addr;
    if(!res)
        return RequestManager::string();
    if(res->sa_family == AF_INET)
    {
        sockaddr_in* addr4 = reinterpret_cast<sockaddr_in*>(res);
        return boost_inet_ipv6_v4(addr4);
    }
    else if(res->sa_family == AF_INET6)
    {
        sockaddr_in6* addr6 = reinterpret_cast<sockaddr_in6*>(res);
        return boost_inet_ipv6(addr6);
    }
    return RequestManager::string();
}



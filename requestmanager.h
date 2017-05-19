#ifndef REQUESTMANAGER_H
#define REQUESTMANAGER_H
#include <microhttpd.h>
#include <list>
#include <string>
#include <limits>

class ContentManager;
class RequestManager;

struct ReqMgrPtr {
    RequestManager* m_owner;
public:
    inline ReqMgrPtr(RequestManager* p)
        : m_owner(p) {}
    
    ReqMgrPtr() =delete;
    
    ContentManager* ContentMgr();
    MHD_Connection* Connection();
    MHD_Response* Response();
    MHD_Daemon* Daemon();
    
    bool SetResponse(MHD_Response*);
};

class RequestManager {
public:
    enum KVKind {
        KVK_RESPONSE_HEADER = MHD_RESPONSE_HEADER_KIND,
        KVK_HEADER = MHD_HEADER_KIND,
        KVK_COOKIE = MHD_COOKIE_KIND,
        KVK_POSTDATA = MHD_POSTDATA_KIND,
        KVK_URI = MHD_GET_ARGUMENT_KIND,
        KVK_FOOTER = MHD_FOOTER_KIND
    };
    
    typedef std::list<std::string> stringlist;
    typedef std::pair<std::string, std::string> stringpair;
    typedef std::string string;
    
    struct KVQueryResult {
        inline KVQueryResult(string k = string(),
                              string v = string(),
                              KVKind kvk = KVK_HEADER)
            : m_key(std::move(k)), m_value(std::move(v)), m_kind(kvk) {}
        string m_key;
        string m_value;
        KVKind m_kind;
    };
    
    typedef std::list<KVQueryResult> KVQueryResults;
    
private:
    friend class ReqMgrPtr;
    
    ContentManager* m_manager;
    MHD_Daemon* m_daemon;
    MHD_Connection* m_connection;
    MHD_Response* m_response;
    char const* m_url;
    char const* m_method;
    char const* m_version;
    char const* m_uploadData;
    size_t m_uploadDataSize;
    
    struct KeyValuesT : public ReqMgrPtr {
        using ReqMgrPtr::ReqMgrPtr;
        
        KVQueryResults GetValues(KVKind);
        string GetValue(KVKind, char const*);
        bool PushValue(KVKind, char const*, char const*);
    };
    
    struct BasicAuthT : public ReqMgrPtr {
        using ReqMgrPtr::ReqMgrPtr;
        
        stringpair GetLoginData();
    };
    
    struct DigestAuthT : public ReqMgrPtr {
        using ReqMgrPtr::ReqMgrPtr;
        
        string GetUsername();
        bool CheckAuth(char const* realm, char const* user, char const* passwd, unsigned int nonce_timeout = 60);
    };
    
    struct ResponseT : public ReqMgrPtr {
        using ReqMgrPtr::ReqMgrPtr;
        
        bool Cancel();
        bool SSendData(char const* data, std::uint64_t size);
        bool SSendData(std::string const&);
        bool SSendFile(char const* path, std::size_t from = 0, std::size_t length = std::numeric_limits<std::size_t>::max());
        
        bool AddHeader(char const* header, char const* content);
        bool AddFooter(char const* footer, char const* content);
        bool RemHeader(char const* header, char const* content = nullptr);
        
        // Use either one to enqueue the response
        bool QRespond(unsigned int code);
        bool QBasicLoginDataRequired(char const* realm);
        bool QDigestLoginDataRequired(char const* realm, char const* opaque, bool stale);
    };
    
    struct ServerInfoT : public ReqMgrPtr {
        using ReqMgrPtr::ReqMgrPtr;
        
        unsigned int CurrentConnections();
    };
    
    struct ConnectionInfoT : public ReqMgrPtr {
        using ReqMgrPtr::ReqMgrPtr;
        
        bool HTTPS();
        
        // https://www.tutorialspoint.com/unix_sockets/socket_structures.htm
        bool IPV4();
        bool IPV6();
        string GetIP();
        string GetIPV6();
    };
    
    KeyValuesT KeyValues;
    BasicAuthT BasicAuth;
    DigestAuthT DigestAuth;
    ResponseT Response;
    ServerInfoT ServerInfo;
    ConnectionInfoT ConnectionInfo;
    
    bool ProcessLua();
public:
    RequestManager(ContentManager*, MHD_Daemon*, MHD_Connection*,
                   char const*, char const*, char const*, char const*, size_t);
    bool Process();
};

#endif // REQUESTMANAGER_H

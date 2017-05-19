#include "requestmanager.h"

ContentManager* ReqMgrPtr::ContentMgr() { return m_owner->m_manager; }
MHD_Connection* ReqMgrPtr::Connection() { return m_owner->m_connection; }
MHD_Response* ReqMgrPtr::Response() { return m_owner->m_response; }
bool ReqMgrPtr::SetResponse(MHD_Response* response) { m_owner->m_response = response; return (response != nullptr); }
MHD_Daemon* ReqMgrPtr::Daemon() { return m_owner->m_daemon; }

RequestManager::RequestManager(ContentManager* manager,
                               MHD_Daemon* daemon,
                               MHD_Connection* connection,
                               const char* url,
                               const char* method,
                               const char* version,
                               const char* uploadData,
                               size_t uploadDataSize)
    : m_manager(manager), m_daemon(daemon),
      m_connection(connection),
      m_response(nullptr), m_url(url),
      m_method(method), m_version(version),
      m_uploadData(uploadData), m_uploadDataSize(uploadDataSize),
      KeyValues(this),
      BasicAuth(this),
      DigestAuth(this),
      Response(this),
      ServerInfo(this),
      ConnectionInfo(this)
{}

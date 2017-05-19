#include "requestmanager.h"
#include "contentmanager.h"
#include <sstream>

bool RequestManager::Process()
{
    std::ostringstream oss;
    oss << "IPV4 : " << ConnectionInfo.IPV4()    << std::endl
        << "IPV6 : " << ConnectionInfo.IPV6()    << std::endl
        << "IP   : " << ConnectionInfo.GetIP()   << std::endl
        << "IPV6 : " << ConnectionInfo.GetIPV6() << std::endl
        << "HTTPS: " << ConnectionInfo.HTTPS()   << std::endl
        << "Conns: " << ServerInfo.CurrentConnections() << std::endl;
    
    return
        this->Response.SSendData(oss.str()) &&
        this->Response.QRespond(200);
}

bool RequestManager::ProcessLua()
{
    // AutoVirtualState vs(this->m_manager);
    // Lua::State& state = vs.State();
    return true;
}

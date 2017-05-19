#include "requestmanager.h"

unsigned int RequestManager::ServerInfoT::CurrentConnections()
{
    MHD_DaemonInfo const* info =
        MHD_get_daemon_info(Daemon(), MHD_DAEMON_INFO_CURRENT_CONNECTIONS);
    return info ? info->num_connections : 1;
}

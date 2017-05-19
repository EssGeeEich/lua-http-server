#include "requestmanager.h"

RequestManager::string RequestManager::DigestAuthT::GetUsername()
{
    char* name = MHD_digest_auth_get_username(Connection());
    if(name)
    {
        string str = name;
        ::free(name);
        return str;
    }
    return string();
}

bool RequestManager::DigestAuthT::CheckAuth(char const* realm, char const* user, char const* passwd, unsigned int nonce_timeout)
{
    return MHD_digest_auth_check(Connection(), realm, user, passwd, nonce_timeout) == MHD_YES;
}

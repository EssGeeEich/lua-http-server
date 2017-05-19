#include "requestmanager.h"

RequestManager::stringpair RequestManager::BasicAuthT::GetLoginData()
{
    RequestManager::stringpair pair;
    char* password = nullptr;
    char* name = MHD_basic_auth_get_username_password(Connection(), &password);
    
    if(name)
    {
        pair.first = name; 
        ::free(name);
    }
    if(password)
    {
        pair.second = password;
        ::free(password);
    }
    return pair;
}

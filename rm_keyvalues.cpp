#include "requestmanager.h"

int SaveKeyValues(void* cls, MHD_ValueKind k, char const* key, char const* val)
{
    RequestManager::KVQueryResults* kvqs = static_cast<RequestManager::KVQueryResults*>(cls);
    kvqs->emplace_back(key,val,static_cast<RequestManager::KVKind>(k));
    return MHD_YES;
}

RequestManager::KVQueryResults RequestManager::KeyValuesT::GetValues(KVKind where)
{
    RequestManager::KVQueryResults results;
    MHD_get_connection_values(Connection(), static_cast<MHD_ValueKind>(where), SaveKeyValues, static_cast<void*>(&results));
    return results;
}

RequestManager::string RequestManager::KeyValuesT::GetValue(KVKind k, char const* key)
{
    return MHD_lookup_connection_value(Connection(), static_cast<MHD_ValueKind>(k), key);
}

bool RequestManager::KeyValuesT::PushValue(KVKind k, char const* key, char const* val)
{
    return MHD_set_connection_value(Connection(), static_cast<MHD_ValueKind>(k), key, val) == MHD_YES;
}

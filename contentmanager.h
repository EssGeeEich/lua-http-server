#ifndef CONTENTMANAGER_H
#define CONTENTMANAGER_H
#include <microhttpd.h>
#include <atomic>
#include <thread>
#include <list>
#include "virtualstate.h"
#include "sequencemanager.h"

enum LuaFallbackBehavior {
    LFB_FIND_LUA_STATE = 0,
    LFB_CREATE_LUA_STATE
};

class ContentManager {
    bool m_alive;
    
    LuaFallbackBehavior const m_luaFb;
    std::list<RealState> m_luaStates;
    SequenceManager m_luaSequenceManager;
    
    void InitLuaState(Lua::State&);
protected:
    friend class RequestManager;
    friend class AutoVirtualState;
    
    VirtualState* LockLuaState();
    void UnlockLuaState(VirtualState*);
    void Kill();
public:
    ContentManager(std::uint16_t lua_precache, LuaFallbackBehavior lua_fb);
    bool HandleRequest(MHD_Daemon*, MHD_Connection*, char const*, char const*,
                       char const*, char const*, size_t);
    bool Alive();
};

class AutoVirtualState {
    ContentManager* m_manager;
    VirtualState* m_state;
public:
    AutoVirtualState(ContentManager* manager);
    ~AutoVirtualState();
    void unlock();
    Lua::State& State();
};

#endif // CONTENTMANAGER_H

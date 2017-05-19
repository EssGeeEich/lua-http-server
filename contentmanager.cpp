#include "contentmanager.h"
#include "requestmanager.h"

ContentManager::ContentManager(std::uint16_t lua_precache, LuaFallbackBehavior lua_fb)
    : m_alive(true),
      m_luaFb(lua_fb)
{
    for(std::uint16_t i = 0; i < lua_precache; ++i)
    {
        m_luaStates.emplace_back();
    }
    
    for(auto it = m_luaStates.begin(); it != m_luaStates.end(); ++it)
    {
        RealState& hs = *it;
        InitLuaState(hs.State());
    }
}

void ContentManager::InitLuaState(Lua::State&)
{
    
}

VirtualState* ContentManager::LockLuaState()
{
    for(auto it = m_luaStates.begin(); it != m_luaStates.end(); ++it)
    {
        RealState& hs = *it;
        if(hs.trylock())
            return &hs;
    }
    
    if(m_luaFb == LFB_FIND_LUA_STATE)
    {
        // Get our ticket
        std::uint64_t ticket = m_luaSequenceManager.GetTicket();
        bool inLoop = true;
        
        // Focus on our duty
        while(true) {
            // Is our ticket the next one yet?
            if(m_luaSequenceManager.ProcessTicket(ticket))
            {
                // It is! But we have to wait for a lua state to free...
                while(inLoop) {
                    for(auto it = m_luaStates.begin(); it != m_luaStates.end(); ++it)
                    {
                        RealState& hs = *it;
                        if(hs.trylock())
                        {
                            // Found it!
                            // The next ticket can start searching now...
                            if(!m_luaSequenceManager.ValidateTicket(ticket))
                            {
                                // But if something had gone wrong,
                                // we must go back to the end of the line.
                                hs.unlock();
                                ticket = m_luaSequenceManager.GetTicket();
                                inLoop = false;
                                break;
                            }
                            return &hs;
                        }
                    }
                    // Sleep for a while...
                    std::this_thread::yield();
                }
                inLoop = true;
            }
            else
            {
                // Sleep for a while...
                std::this_thread::yield();
            }
        }
    }
    else if(m_luaFb == LFB_CREATE_LUA_STATE)
    {
        std::unique_ptr<VirtualState> vs(new TempLocklessState);
        InitLuaState(vs->State());
        return vs.release();
    }
    return nullptr;
}

void ContentManager::UnlockLuaState(VirtualState* hs)
{
    hs->unlock();
}

bool ContentManager::HandleRequest(MHD_Daemon* daemon,
    MHD_Connection* connection, char const* url, char const* method,
    char const* version, char const* uploadData, size_t uploadDataSize)
{
    return RequestManager(this, daemon, connection, url, method,
                          version, uploadData, uploadDataSize).Process();
}

bool ContentManager::Alive()
{
    return m_alive;
}

void ContentManager::Kill()
{
    m_alive = false;
}

AutoVirtualState::AutoVirtualState(ContentManager* manager)
    : m_manager(manager), m_state(manager ? manager->LockLuaState() : nullptr) {}

AutoVirtualState::~AutoVirtualState() { unlock(); }
void AutoVirtualState::unlock() {
    if(m_manager && m_state)
    {
        m_manager->UnlockLuaState(m_state);
        m_manager = nullptr;
        m_state = nullptr;
    }
}
Lua::State& AutoVirtualState::State()
{
    if(!m_state)
        throw std::runtime_error("State requested after release.");
    return m_state->State();
}

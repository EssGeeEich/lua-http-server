#ifndef VIRTUALSTATE_H
#define VIRTUALSTATE_H
#include "state.h"
#include "spinlock_mutex.h"

class ContentManager;

class VirtualState {
    friend class ContentManager;
    Lua::State m_state;
    
    virtual bool trylock() =0;
    virtual void lock() =0;
    virtual void unlock() =0;
public:
    virtual ~VirtualState();
    Lua::State& State();
};

class RealState : public VirtualState {
    friend class ContentManager;
    spinlock_mutex m_lock;
    
    bool trylock() override;
    void lock() override;
    void unlock() override;
};

class TempLocklessState : public VirtualState {
    friend class ContentManager;
    
    bool trylock() override;
    void lock() override;
    void unlock() override; // will "delete this;"
    
    TempLocklessState();
};

#endif // VIRTUALSTATE_H

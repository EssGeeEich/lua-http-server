#include "virtualstate.h"

VirtualState::~VirtualState()          {             }
TempLocklessState::TempLocklessState() {             }
Lua::State& VirtualState::State()  { return m_state; }

bool RealState::trylock() { return m_lock.trylock(); }
void RealState::lock()    { return m_lock.   lock(); }
void RealState::unlock()  { return m_lock. unlock(); }

bool TempLocklessState::trylock() { return true; }
void TempLocklessState::lock()    {              }
void TempLocklessState::unlock()  { delete this; }

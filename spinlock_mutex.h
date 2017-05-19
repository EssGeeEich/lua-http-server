#ifndef SPINLOCK_MUTEX_H
#define SPINLOCK_MUTEX_H
#include <atomic>

class spinlock_mutex {
    std::atomic_flag m_flag;
public:
    inline spinlock_mutex() noexcept :
        m_flag(ATOMIC_FLAG_INIT) {}
    inline void lock() noexcept {
        while(m_flag.test_and_set(std::memory_order_acquire))
            ;
    }
    bool trylock() noexcept {
        return m_flag.test_and_set(std::memory_order_acquire) == 0;
    }
    void unlock() noexcept {
        m_flag.clear(std::memory_order_release);
    }
};

#endif // SPINLOCK_MUTEX_H

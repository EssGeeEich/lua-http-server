#ifndef SEQUENCEMANAGER_H
#define SEQUENCEMANAGER_H
#include <atomic>
#include <cstdint>

class SequenceManager {
    std::atomic_ullong m_currentTicket;
    std::atomic_ullong m_lastTicket;
public:
    SequenceManager();
    std::uint64_t GetTicket();
    bool ProcessTicket(std::uint64_t);
    bool ValidateTicket(std::uint64_t);
};

#endif // SEQUENCEMANAGER_H

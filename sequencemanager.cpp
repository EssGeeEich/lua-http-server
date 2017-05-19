#include "sequencemanager.h"

SequenceManager::SequenceManager()
    : m_currentTicket(0), m_lastTicket(0) {}

std::uint64_t SequenceManager::GetTicket()
{
    return m_lastTicket.fetch_add(1, std::memory_order_relaxed);
}

bool SequenceManager::ProcessTicket(std::uint64_t ticket)
{
    return m_currentTicket.load() == ticket;
}

bool SequenceManager::ValidateTicket(std::uint64_t ticket)
{
    return m_currentTicket.compare_exchange_strong(ticket, ticket+1);
}

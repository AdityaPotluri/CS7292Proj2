#pragma once
#include <cstdint>

struct Range
{
    uint32_t m_start;
    uint32_t m_end;

    Range(uint32_t start, uint32_t end): m_start(start), m_end(end) {}
    
    Range(): m_start(0), m_end(0) {}

    bool contains(uint32_t idx)
    {
        return idx >= m_start and idx < m_end;
    }

    uint32_t start()
    {
        return m_start;
    }

    uint32_t end()
    {
        return m_end;
    }

};

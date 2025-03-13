#pragma once
#include <vector>
#include <deque>
#include <array>
#include <cstdint>
#include <algorithm>
#include <random>

#include "cacheline.hpp"


class EvictionSet
{
public:
    EvictionSet(std::vector<void*>& cache_lines): m_cachelines(cache_lines)
    {
        static_assert(sizeof(CacheLine) == 64, "Size of CacheLine is not 64B.");
    }

    EvictionSet(std::vector<void*>::iterator first, std::vector<void*>::iterator last): m_cachelines(first, last)
    {
        static_assert(sizeof(CacheLine) == 64, "Size of CacheLine is not 64B.");
    }

    EvictionSet(std::deque<void*>::iterator first, std::deque<void*>::iterator last): m_cachelines(first, last)
    {
        static_assert(sizeof(CacheLine) == 64, "Size of CacheLine is not 64B.");
    }
    
    EvictionSet()
    {
        static_assert(sizeof(CacheLine) == 64, "Size of CacheLine is not 64B.");
    }
    ~EvictionSet() = default;
    
    // replace cache lines
    void replace_cachelines(std::vector<void*>&);
    
    // build doubly linked list
    void build_dll();
    
    // Traverse the doubly linked list
    void traverse();
                     
    // Load the target and followed by a traversal to the LL once
    template<typename Timer>
    uint64_t evict_and_time_once(const void* target);

    // Calls evict_and_time_once multiple times to get rid of any outliers
    template<typename Timer, std::size_t SAMPLES>
    uint64_t evict_and_time(std::array<uint64_t, SAMPLES>& timings, const void* target);

private:
    // All the cachelines should be atleast 64B
    std::vector<void*> m_cachelines;

    // Doubly Linked List
    DLL m_dll;
};

#include <iostream>
#include <thread>

#include "eviction_set.h"
#include "timer.h"

void EvictionSet::replace_cachelines(std::vector<void*>& cache_lines)
{
    m_cachelines = cache_lines;
}

void EvictionSet::build_dll()
{
    m_dll.clean();
    for (auto e: m_cachelines)
    {
        m_dll.insert(CacheLine::get_item(e));
    }
}

// Added the attribute -O0 to not optmize the traveral code when running with -O3
#ifdef __APPLE__
void __attribute__((optnone)) EvictionSet::traverse()
#else
void __attribute__((optimize("O0"))) EvictionSet::traverse()
#endif
{
    for (int i = 0; i < 2; i++)
    {
        CacheLine* head = m_dll.get_head();
        CacheLine* lagging_head = m_dll.get_head();

        for (int j = 0; j < 8 and head != nullptr; j++)
        {
            head = head->next();
        }

        while (head != nullptr and head->next() != nullptr)
        {
            lagging_head = lagging_head->next();
            head = head->next();
        }
        
        while (lagging_head != nullptr and lagging_head->next() != nullptr)
        {
            lagging_head = lagging_head->next();
        }

        CacheLine* tail = m_dll.get_tail();
        CacheLine* lagging_tail = m_dll.get_tail();

        for (int j = 0; j < 8 and tail != nullptr; j++)
        {
            tail = tail->prev();
        }

        while (tail != nullptr and tail->prev() != nullptr)
        {
            lagging_tail = lagging_tail->prev();
            tail = tail->prev();
        }
        
        while (lagging_tail != nullptr and lagging_tail->prev() != nullptr)
        {
            lagging_tail = lagging_tail->prev();
        }
    }
}

template<typename Timer>
uint64_t EvictionSet::evict_and_time_once(const void* target)
{
    // Step 1: Load the target
    *reinterpret_cast<const volatile uint8_t*>(target); 
   
    // Step 2: Traverse the Linked List
    traverse();
    
    // Load and time the target
    return Timer::time_load(target);
}

template<typename Timer, std::size_t SAMPLES>
uint64_t EvictionSet::evict_and_time(std::array<uint64_t, SAMPLES>& timings, const void* target)
{
    auto rd = std::random_device {}; 
    auto rng = std::default_random_engine { rd() };

    for (int idx = 0; idx < timings.size(); idx++)
    {
        std::shuffle(m_cachelines.begin(), m_cachelines.end(), rng);
    
        build_dll();
        
        timings[idx] = evict_and_time_once<Timer>(target);
    }
    
    std::sort(timings.begin(), timings.end());

    return timings[timings.size() / 2];
}



template uint64_t EvictionSet::evict_and_time<RdtscTimer, 100>(std::array<uint64_t, 100>&, const void*);
template uint64_t EvictionSet::evict_and_time_once<RdtscTimer>(const void*);

#include <iostream>
#include <sys/mman.h>
#include <fstream>
#include <chrono>
#include <x86intrin.h>

// local include files
#include "eviction_set.h"
#include "allocator.h"
#include "victim.h"
#include "timer.h"

#include "utils.h"

constexpr uint32_t SAMPLES = 100;
constexpr uint32_t THRESHOLD = 150;
constexpr PROCESSOR CPU = INTEL_CORE_I7_8700K;
constexpr uint32_t WAYS = get_llc_ways(CPU);
constexpr uint32_t STRIDE = PAGE_SIZE_4K;

std::vector<void*> find_eviction_set(Allocator& allocator, void* victim)
{
    uint64_t time;
    std::array<uint64_t, SAMPLES> timings;

    std::vector<void*> eviction_set = allocator.inflate<RdtscTimer, SAMPLES>(victim, 16384, THRESHOLD);

    EvictionSet evset(eviction_set);
    time = evset.evict_and_time<RdtscTimer>(timings, victim);
    std::cout << "Evicted: " << time << std::endl;

    allocator.reduce<RdtscTimer, SAMPLES>(eviction_set, victim, THRESHOLD, WAYS);
    evset.replace_cachelines(eviction_set);

    uint32_t count = 0;
    for (int i = 0; i < SAMPLES; i++)
    {
        time = evset.evict_and_time<RdtscTimer>(timings, victim);
        if (time >= THRESHOLD)
        {
            count += 1;
        }
    }

    if (count == SAMPLES and eviction_set.size() == WAYS)
    {
        dump_evset(victim, eviction_set, CPU);
    }
    else
    {
        fprintf(stderr, "Unable to find eviction set for %p\n", victim);
        return std::vector<void*>();
    }   
    return eviction_set;
}


int main (int argc, char** argv)
{
    uint64_t offset = 256;
    Victim victim_helper(offset, STRIDE);
    Allocator allocator(offset, STRIDE);
    std::vector<void*> evset_elem;
    void* victim;

    while (evset_elem.size() != WAYS)
    {
        victim = victim_helper.find_victim();
        evset_elem = find_eviction_set(allocator, victim);
    }

    std::array<uint64_t, 100> timings;

    for (int sz = 1; sz <= evset_elem.size(); sz++)
    {
        printf("\nSize = %d\n", sz);
        std::vector<void*> sub(evset_elem.begin(), evset_elem.begin() + sz);
        EvictionSet evset(sub);

        for (int i = 0; i < 5; i++)
        {

            uint64_t time = evset.evict_and_time<RdtscTimer>(timings, victim);
            std::cout << "Evict Time: " << time << std::endl;    
        }
    }

    return 0;
}

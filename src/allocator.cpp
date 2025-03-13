#include <iostream>
#include <cstring>
#include <stdexcept>
#include <sys/mman.h>
#include <deque>

#include "allocator.h"
#include "eviction_set.h"
#include "timer.h"

#include "range.h"


void Allocator::allocate_page()
{
    void *page = mmap(NULL, PAGE_SIZE_4K, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1,  0);
    if (page == MAP_FAILED)
    {
        std::cerr << "Unable to allocate a new page";
        return;
    }
    
    memset(page, 0xff, PAGE_SIZE_4K);
    
    for (int i = 0; i < PAGE_SIZE_4K; i += m_stride)
    {
        m_cachelines.push_back(reinterpret_cast<uint8_t*>(page) + i + m_offset);
    }

    m_pages.push_back(page);
}

void Allocator::allocate_hugepage()
{

    void *page = mmap(NULL, PAGE_SIZE_2M, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1,  0);
    if (page == MAP_FAILED)
    {
        std::cerr << "Unable to allocate a huge page\n";
        return;
    }
    
    memset(page, 0xff, PAGE_SIZE_2M);
    
    for (int i = 0; i < PAGE_SIZE_2M; i += m_stride)
    {
        m_cachelines.push_back(reinterpret_cast<uint8_t*>(page) + i + m_offset);
    }

    m_hugepages.push_back(page);
}


Allocator::~Allocator()
{
    for (auto page: m_pages)
    {
        munmap(page, PAGE_SIZE_4K);
    }

    for (auto page: m_hugepages)
    {
        munmap(page, PAGE_SIZE_2M);
    }

    // Done this to be safe
    m_pages.clear();
    m_cachelines.clear();
}

void* Allocator::allocate_line()
{
    void* retval;
    if (not m_cachelines.empty())
    {
        retval = m_cachelines.back();
        m_cachelines.pop_back();
        return retval;
    }

    // allocate_hugepage();
    allocate_page();

    if (not m_cachelines.empty())
    {
        retval = m_cachelines.back();
        m_cachelines.pop_back();
    }
    else
    {
        std::cerr << "Unable to allocate new line\n";
        throw std::runtime_error("Unable to allocate new line\n");
    }

    return retval;
}

// This function is used to replenish the m_cachelines struct, helpful when finding multiple eviction sets
void Allocator::set_offset(uint64_t offset)
{
    printf("Allocator::set_offset(%lu)\n", offset);
    m_offset = offset;
    m_cachelines.clear();

    for (auto page: m_pages)
    {
        for (int i = 0; i < PAGE_SIZE_4K; i += m_stride)
        {
            m_cachelines.push_back(reinterpret_cast<uint8_t*>(page) + i + m_offset);
        }
    }

    for (auto page: m_hugepages)
    {
        for (int i = 0; i < PAGE_SIZE_4K; i += m_stride)
        {
            m_cachelines.push_back(reinterpret_cast<uint8_t*>(page) + i + m_offset);
        }
    }

}

// This function finds a set of cache lines that can be used to evict the target
// from the LLC.
// Return a vector of void pointers that can be used to evict the target.
// To allocate a new cache line, use the allocate_line function.
// To evict the target, use the EvictionSet class.
// Initialize the eviction set with the vector of void pointers.
// Use the evict_and_time function to evict the target and measure the time.
// If the time is greater than the threshold, then the eviction set is valid.
template<typename Timer, std::size_t SAMPLES>
std::vector<void*> Allocator::inflate(void* target, uint32_t max_size, uint64_t threshold)
{
    printf("Allocator::inflate(max_size = %u, threshold = %lu)\n", max_size, threshold);
    std::vector<void*> retval;

    // TODO: Implement the inflate function
    while ( retval.size() < max_size ) {
        void* new_line = allocate_line();

        retval.push_back(new_line);
        EvictionSet eviction_set(retval);

        std::array<uint64_t, SAMPLES> timings;
        eviction_set.evict_and_time<Timer, SAMPLES>(timings, target);

        bool valid = std::all_of(timings.begin(), timings.end(),
                        [threshold](auto t) { return t >= threshold; });

        if (valid) {
            break;
        }
    }

    return retval;
}

// This function reduces the number of cache lines in the eviction set to the
// number of ways in the LLC.
// You have to use the algorithm described in the paper Theory and Practice of Finding Eviction Sets to find a minimal eviction set.
template<typename Timer, std::size_t SAMPLES>
void Allocator::reduce(std::vector<void*>& cache_lines, void* target, uint64_t threshold, uint32_t ways)
{
    printf("Allocator::reduce(cache_lines.size() = %lu, threshold = %lu, ways = %u)\n", cache_lines.size(), threshold, ways);
    /*
        Algorithm 2 Reduction Via Group Testing
        In : S=candidate set, x=victim address
        Out : R=minimal eviction set for x
        1: while |S| > a do
        2: {T1, ..., Ta+1} ← split(S, a + 1)
        3: i ← 1
        4: while ¬TEST(S \ Ti
        , x) do
        5: i ← i + 1
        6: end while
        7: S ← S \ Ti
        8: end while
        9: return S
    */

    // while |S| > a 
    while (cache_lines.size() > ways) {
        // const size_t l = ways + 1;
        // if (cache_lines.size() < l) break;

        //  2: {T1, ..., Ta+1} ← split(S, a + 1)
        std::vector<std::pair<size_t, size_t>> group_ranges;
        const size_t base_size = cache_lines.size() / l;
        const size_t remainder = cache_lines.size() % l;
        size_t current_idx = 0;

        // we use a base + offset datastructure for the group ranges, the remainder is put in the first couple buckets
        for (size_t i = 0; i < l; ++i) {
            const size_t group_size = base_size + (i < remainder ? 1 : 0);
            group_ranges.emplace_back(current_idx, group_size);
            current_idx += group_size;
        }

        bool removed = false;
        for (const auto& [start, size] : group_ranges) {
            
            std::vector<void*> reduced_set;
            reduced_set.reserve(cache_lines.size() - size);

            
            if (start > 0) {
                reduced_set.insert(reduced_set.end(), 
                                 cache_lines.begin(), 
                                 cache_lines.begin() + start);
            }

            
            const size_t end = start + size;
            if (end < cache_lines.size()) {
                reduced_set.insert(reduced_set.end(),
                                 cache_lines.begin() + end,
                                 cache_lines.end());
            }

           
            EvictionSet tester(reduced_set);
            tester.build_dll(); 
            
            std::array<uint64_t, SAMPLES> timings;
            tester.evict_and_time<Timer, SAMPLES>(timings, target);

          
            bool valid = std::all_of(timings.begin(), timings.end(),
                                   [threshold](auto t) { return t >= threshold; });

            if (valid) {
                cache_lines = std::move(reduced_set);
                removed = true;
                break;  
            }
        }

        if (!removed) break; 
    }


}


// Similar to the reduce function, but uses few tricks to speed up the process of finding the minimal eviction set.
// The tricks are described in the paper iLeakage: Browser-based Timerless Speculative Execution Attacks on Apple Devices, section 4.4
// You don't have to implement this function, but you can get extra points if you do.
template<typename Timer, std::size_t SAMPLES>
void Allocator::reduce2(std::vector<void*>& cache_lines, void* target,
                            uint64_t threshold, uint32_t ways)
{
    printf("Allocator::reduce2(cache_lines.size() = %lu, threshold = %lu, ways = %u)\n", cache_lines.size(), threshold, ways);
} 


template std::vector<void*> Allocator::inflate<RdtscTimer, 100>(void*, uint32_t, uint64_t);
template void Allocator::reduce<RdtscTimer, 100>(std::vector<void*>&, void*, uint64_t, uint32_t);
template void Allocator::reduce2<RdtscTimer, 100>(std::vector<void*>&, void*, uint64_t, uint32_t);
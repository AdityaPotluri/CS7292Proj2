#pragma once
#include <cstdint>
#include <vector>

#include "mmap_constants.h"

class Allocator
{
public:
    // offset: This is the offset within a page from stride
    // stride: This is helpful when using hugepages. It is usually 4K 
    Allocator(uint64_t offset, uint64_t stride): m_offset(offset), m_stride(stride) {}

    // All the pages in m_pages should be unmaped and m_pages and m_cachelines should be emptied
    ~Allocator();

    // Can be used to modify the offset of cache lines within a page
    void set_offset(uint64_t offset);

    template<typename Timer, std::size_t SAMPLES>
    std::vector<void*> inflate(void* target, uint32_t max_size, uint64_t threshold);

    // Pepe Villa
    // Input: Eviction Set from Inflate
    // Output: Minimal Eviction Set
    // Done In-place
    template<typename Timer, std::size_t SAMPLES>
    void reduce(std::vector<void*>& cache_lines, void* target, uint64_t threshold, uint32_t ways);

    // from iLeakage
    template<typename Timer, std::size_t SAMPLES>
    void reduce2(std::vector<void*>& cache_lines, void* target, uint64_t threshold, uint32_t ways);

private:
    void allocate_page();
    void allocate_hugepage();
    void* allocate_line();

    uint64_t m_offset;
    uint64_t m_stride;
    std::vector<void*> m_pages;
    std::vector<void*> m_hugepages;
    std::vector<void*> m_cachelines;
};

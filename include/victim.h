#pragma once
#include <cstdint>
#include <vector>

#include "mmap_constants.h"

class Victim
{
public:
    // offset: This is the offset within a page from stride
    // stride: This is helpful when using hugepages. It is usually 4K 
    Victim(uint64_t offset, uint64_t stride): m_offset(offset), m_stride(stride) {}

    // All the pages in m_pages should be unmaped and m_pages and m_cachelines should be emptied
    ~Victim();

    // Can be used to modify the offset of cache lines within a page
    void set_offset(uint64_t offset);

    void* find_victim();
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

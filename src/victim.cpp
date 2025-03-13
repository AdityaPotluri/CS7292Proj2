#include <iostream>
#include <cstring>
#include <stdexcept>
#include <sys/mman.h>

#include "victim.h"
#include "eviction_set.h"
#include "timer.h"

#include "range.h"


void Victim::allocate_page()
{
    void *page = mmap(NULL, PAGE_SIZE_4K, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1,  0);
    if (page == MAP_FAILED)
    {
        std::cerr << "Unable to allocate a new page";
        return;
    }
    
    memset(page, 0xff, PAGE_SIZE_4K);
    
    for (int i = 0; i < PAGE_SIZE_4K; i += m_stride)
    {
        m_cachelines.push_back(reinterpret_cast<uint8_t*>(page) + uint64_t(i + m_offset));
    }

    m_pages.push_back(page);
}

void Victim::allocate_hugepage()
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
        m_cachelines.push_back(reinterpret_cast<uint8_t*>(page) + uint64_t(i + m_offset));
    }

    m_hugepages.push_back(page);
}


Victim::~Victim()
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

void* Victim::allocate_line()
{
    void* retval;
    if (not m_cachelines.empty())
    {
        retval = m_cachelines.back();
        m_cachelines.pop_back();
        return retval;
    }

    //allocate_hugepage();
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
void Victim::set_offset(uint64_t offset)
{
    printf("Victim::set_offset(%lu)\n", offset);
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

    auto rd = std::random_device {};
    auto rng = std::default_random_engine { rd() };

    std::shuffle(m_cachelines.begin(), m_cachelines.end(), rng);
 

    printf("m_cachelines.size() = %lu\n", m_cachelines.size());
}


void* Victim::find_victim()
{
    void* victim = allocate_line();
    return victim;
}
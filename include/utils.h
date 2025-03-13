#pragma once
#include <cstdint>
#include <vector>
#include <bitset>
#include <chrono>

#include "pagemap.h"
#include "processor.h"


size_t parity(uint64_t value)
{
    return std::bitset<64>(value).count() & 1;
}

size_t get_cache_slice(uint64_t addr, const std::vector<size_t>& sequence, const std::vector<uint64_t>& masks)
{
    size_t index_mask = (1ULL << masks.size()) - 1;
    size_t index = ((addr >> 6) & index_mask);

    for (size_t i = 0; i < masks.size(); i++) {
        index ^= parity(addr & masks[i]) << i;
    }

    return sequence[index];
}

size_t get_cache_set(uint64_t addr, uint32_t size)
{
    return (addr >> 6) & (size - 1);
}

PageMap self_pm = PageMap::with_self();

uint64_t get_phy_addr(uint64_t va)
{
    PageTableEntry pte = self_pm.read(va);
    uint64_t pg_off = va & 0xfff;
    uint64_t phy_addr = (pte.page_frame_number() << 12) + pg_off;

    return phy_addr;
}

std::tuple<uint64_t, uint32_t, uint32_t> get_set_slice(uint64_t va, PROCESSOR p)
{
    uint64_t phy_addr = get_phy_addr(va);

    uint32_t num_sets = get_llc_sets(p);
    auto slice_func = get_llc_slice_func(p);
    
    uint32_t set_idx = get_cache_set(phy_addr, num_sets);
    uint32_t slice_idx = get_cache_slice(phy_addr, slice_func.first, slice_func.second);

    return std::make_tuple(phy_addr, set_idx, slice_idx);
}

void dump_evset(void* victim, std::vector<void*>& evset, PROCESSOR p)
{
    std::tuple<uint64_t, uint32_t, uint32_t> ret;
    if (victim != nullptr)
    {
        ret = get_set_slice((uint64_t)victim, p);
        printf("Victim: [Phy] = 0x%lx, [Set] = %u, [Slice] = %u\n", std::get<0>(ret), std::get<1>(ret), std::get<2>(ret));
    }
    
    for (auto e: evset)
    {
        ret = get_set_slice((uint64_t)e, p);
        printf("Eveset: [Phy] = 0x%lx, [Set] = %u, [Slice] = %u\n", std::get<0>(ret), std::get<1>(ret), std::get<2>(ret));
    }
}
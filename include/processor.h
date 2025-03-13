#pragma once
#include <cstdint>
#include <vector>

// Constants for Intel Core i7-8700K
const std::vector<size_t> INTEL_CORE_I7_8700K_SEQUENCE = {
    0, 1, 2, 3, 1, 4, 3, 4, 1, 0, 3, 2, 0, 5, 2, 5,
    1, 0, 3, 2, 0, 5, 2, 5, 0, 5, 2, 5, 1, 4, 3, 4,
    0, 1, 2, 3, 5, 0, 5, 2, 5, 0, 5, 2, 4, 1, 4, 3,
    1, 0, 3, 2, 4, 1, 4, 3, 4, 1, 4, 3, 5, 0, 5, 2,
    2, 3, 0, 1, 5, 2, 5, 0, 3, 2, 1, 0, 4, 3, 4, 1,
    3, 2, 1, 0, 4, 3, 4, 1, 4, 3, 4, 1, 5, 2, 5, 0,
    2, 3, 0, 1, 3, 4, 1, 4, 3, 4, 1, 4, 2, 5, 0, 5,
    3, 2, 1, 0, 2, 5, 0, 5, 2, 5, 0, 5, 3, 4, 1, 4
};

const std::vector<uint64_t> INTEL_CORE_I7_8700K_MASKS = {
    0x21ae7be000,
    0x435cf7c000,
    0x2717946000,
    0x4e2f28c000,
    0x1c5e518000,
    0x38bca30000,
    0x50d73de000
};

enum PROCESSOR
{
    INTEL_CORE_I7_8700K
};


constexpr uint32_t get_llc_ways(PROCESSOR p)
{
    switch (p)
    {
        case INTEL_CORE_I7_8700K:
            // TODO: Update this
            return 16;
    }

    return 16;
}


constexpr uint32_t get_llc_sets(PROCESSOR p)
{
    switch (p)
    {
        case INTEL_CORE_I7_8700K:
            // TODO: Update this
            return 1019;
    }

    return 2048;
}

std::pair<std::vector<size_t>, std::vector<uint64_t>> get_llc_slice_func(PROCESSOR p)
{
    switch (p)
    {
        case INTEL_CORE_I7_8700K:
            return std::make_pair(INTEL_CORE_I7_8700K_SEQUENCE, INTEL_CORE_I7_8700K_MASKS);
    }

    return std::make_pair(INTEL_CORE_I7_8700K_SEQUENCE, INTEL_CORE_I7_8700K_MASKS);
}


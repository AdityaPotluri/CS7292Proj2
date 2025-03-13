#include <x86intrin.h>

#include "timer.h"


uint64_t RdtscTimer::time(std::function<void()> f)
{
    uint32_t core_id = 0;

    // Measure the CPU cycles before.
    _mm_mfence();
    uint64_t t0 = __rdtscp(&core_id);
    _mm_lfence();

    f();

    // Measure the CPU cycles after. This waits until the preceding load is
    // globally visible. In addition, we execute LFENCE after RDTSCP to
    // ensure RDTSCP finishes before we execute any code.
    uint64_t t1 = __rdtscp(&core_id);
    _mm_lfence();

    // Return the difference.
    return t1 - t0;
}

uint64_t RdtscTimer::time_load(const void* victim)
{
    uint32_t core_id = 0;

    // Measure the CPU cycles before. RDTSCP ensures that all preceding
    // instructions have been executed and that all loads are globally
    // visible. Therefore, we issue MFENCE first to also serialize any
    // stores. Then we use LFENCE after RDTSCP to ensure RDTSCP finishes
    // before we execute any code.
    _mm_mfence();
    uint64_t t0 = __rdtscp(&core_id);
    _mm_lfence();

    // Perform the load.
    *reinterpret_cast<const volatile uint8_t*>(victim);

    // Measure the CPU cycles after. This waits until the preceding load is
    // globally visible. In addition, we execute LFENCE after RDTSCP to
    // ensure RDTSCP finishes before we execute any code.
    uint64_t t1 = __rdtscp(&core_id);
    _mm_lfence();

    return t1 - t0;
}

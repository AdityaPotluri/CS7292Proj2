#pragma once

#include <cstdint>
#include <functional>
#include <chrono>

struct RdtscTimer
{
    static uint64_t time(std::function<void()> f);
    static uint64_t time_load(const void* victim);
};

inline uint64_t get_current_time_ns()
{
    using namespace std::chrono;

    auto current_time = high_resolution_clock::now();
    auto time_since_epoch = current_time.time_since_epoch();
    auto ns = duration_cast<nanoseconds>(time_since_epoch).count();

    return static_cast<uint64_t>(ns);
}
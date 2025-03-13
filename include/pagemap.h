#pragma once
#include <fcntl.h>
#include <sys/uio.h>
#include <unistd.h>
#include <cstdint>
#include <string>
#include <stdexcept>
#include <system_error>

struct Error : std::exception
{
    Error(std::string message) : message(std::move(message)) {}
    const char* what() const noexcept override { return message.c_str(); }
    const std::string message;
};

struct PageTableEntry
{
    uint64_t n;
    
    static constexpr uint64_t PFN_MASK = (1ull << (54 + 1)) - 1;
    static constexpr size_t SOFT_DIRTY_BIT = 55;
    static constexpr uint64_t SOFT_DIRTY_MASK = (1ull << SOFT_DIRTY_BIT);
    static constexpr size_t SWAPPED_BIT = 62;
    static constexpr uint64_t SWAPPED_MASK = (1ull << SWAPPED_BIT);
    static constexpr size_t PRESENT_BIT = 63;
    static constexpr uint64_t PRESENT_MASK = (1ull << PRESENT_BIT);

    constexpr PageTableEntry(uint64_t n) : n(n) {}
    
    constexpr uint64_t page_frame_number() const
    {
        return n & PFN_MASK;
    }
    
    constexpr bool is_soft_dirty() const
    {
        return (n & SOFT_DIRTY_MASK) != 0;
    }
    
    constexpr bool is_swaped() const
    {
        return (n & SWAPPED_MASK) != 0;
    }
    
    constexpr bool is_present() const
    {
        return (n & PRESENT_MASK) != 0;
    }
};

struct PageMap {
    int fd;
    
    static PageMap with_self()
    {
        int fd = open("/proc/self/pagemap", O_RDONLY);
        if (fd == -1)
        {
            throw std::system_error(errno, std::system_category());
        }
        return {fd};
    }
    
    static PageMap with_pid(pid_t pid)
    {
        int fd = -1;
        try
        {
            std::string path = "/proc/" + std::to_string(pid) + "/pagemap";
         
            fd = open(path.c_str(), O_RDONLY);
            if (fd == -1)
            {
                throw std::system_error(errno, std::system_category());
            }
            return {fd};
        }
        catch (...)
        {
            if (fd != -1)
                close(fd);
            throw;
        }
    }
    
    PageTableEntry read(uint64_t addr) const
    {
        uint64_t offset = (addr >> 12) * 8;
        uint8_t buf[8];
        size_t size = 8;
        size_t nread = 0;
        do
        {
            ssize_t ret = pread(fd, buf + nread, size - nread, offset);
            if (ret == -1)
            {
                break;
            }
            else
            {
                /* some data was read */
                nread += ret;
            }
        }
        while (nread < size);
        
        if (nread != size)
        {
            throw Error("Short read from pagemap");
        }
        uint64_t entry = *reinterpret_cast<uint64_t*>(buf);
        return {entry};
    }
};

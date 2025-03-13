#pragma once
#include <cstdint>
#include <iostream>
#include <cstring>

struct CacheLine
{
    uint64_t pad[6]; // 48 Bytes
    CacheLine* m_prev; // 8 Bytes
    CacheLine* m_next; // 8 bytes

    CacheLine* next()
    {
        return m_next;
    }

    CacheLine* prev()
    {
        return m_prev;
    }

    static CacheLine* get_item(void* addr)
    {
        CacheLine* ret = reinterpret_cast<CacheLine*>(addr);
        ret->m_prev = nullptr;
        ret->m_next = nullptr;
        return ret;
    }
};


class DLL
{
public:
    DLL() : head(nullptr), tail(nullptr) {}

    void insert(CacheLine* node)
    {
        if (head == nullptr)
        {
            // Inserting into an empty list
            head = node;
            tail = node;
        }
        else
        {
            // Insert at the end of the list
            tail->m_next = node;
            node->m_prev = tail;
            tail = node;
        }
    }


    CacheLine* get_head()
    {
        return head;
    }

    CacheLine* get_tail()
    {
        return tail;
    }

    void display()
    {
        CacheLine* current = head;
        while (current != nullptr) {
            printf("%p ->", current);
            current = current->next();
        }
        std::cout << std::endl;
    }

    // DLL doesn't  own any of this memory so we don't unmap anything
    void clean()
    {
        head = nullptr;
        tail = nullptr;
    }

private:
    CacheLine* head;
    CacheLine* tail;
};

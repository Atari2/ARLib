#pragma once
#include "BaseTraits.h"
#include "Assertion.h"
#include "std_includes.h"

namespace ARLib {
    constexpr uint32_t hash32(uint32_t key) {
        key += ~(key << 15);
        key ^= (key >> 10);
        key += (key << 3);
        key ^= (key >> 6);
        key += ~(key << 11);
        key ^= (key >> 16);
        return key;
    }

    constexpr uint32_t hash32d(uint32_t key) {
        key = ~key + (key >> 23);
        key ^= (key << 12);
        key ^= (key >> 7);
        key ^= (key << 2);
        key ^= (key >> 20);
        return key;
    }

    constexpr uint32_t hash32p(uint32_t key1, uint32_t key2) {
        return hash32((hash32(key1) * 209) ^ (hash32(key2 * 413)));
    }

    constexpr uint32_t hash64(uint64_t key) {
        uint32_t first = key & 0xFFFFFFFF;
        uint32_t last = key >> 32;
        return hash32p(first, last);
    }

    constexpr uint32_t hashsize(size_t key) {
        if constexpr (sizeof(size_t) == 8)
            return hash64(key);
        else
            return hash32(key);
    }

    constexpr uint32_t hashptr(PtrSize ptr) {
        if constexpr (sizeof(ptr) == 8)
            return hash64(ptr);
        else if constexpr (sizeof(ptr) == 4)
            return hash32(ptr);
        else
            HARD_ASSERT(false, "Invalid pointer size");
    }

    constexpr uint32_t hashptr(const void* ptr) {
        return hashptr((PtrSize)uintptr_t(ptr));
    }

    constexpr uint32_t hashstr(const char* string, size_t size)
    {
        uint32_t hash = 0;
        for (size_t i = 0; i < size; i++) {
            hash += static_cast<uint32_t>(string[i]);
            hash += (hash << 10);
            hash ^= (hash >> 6);
        }
        hash += hash << 3;
        hash ^= hash >> 11;
        hash += hash << 15;
        return hash;
    }
}
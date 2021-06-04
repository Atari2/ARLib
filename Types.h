#pragma once
#include "Compat.h"

namespace ARLib {
    typedef decltype(nullptr) nullptr_t;
#ifdef WINDOWS
    #if defined(ENVIRON64)
        typedef unsigned __int64 size_t;
        typedef unsigned __int64 uintptr_t;
        typedef __int64          ptrdiff_t;
        typedef __int64          intptr_t;
    #else
#error "This library is 64-bit only"
    #endif
        typedef unsigned __int64 uint64_t;
        typedef unsigned __int32 uint32_t;
        typedef unsigned __int16 uint16_t;
        typedef unsigned __int8 uint8_t;
#else
    // we fall here if we're using MINGW or we're on Linux
#define SIZED decltype(sizeof(void*))
    #if defined(ENVIRON64)
        static_assert(sizeof(long long) == sizeof(void*));
        typedef SIZED size_t;
        typedef SIZED uintptr_t;
        typedef long long          ptrdiff_t;
        typedef long long          intptr_t;
    #else
#error "This library is 64-bit only"
    #endif
    static_assert(sizeof(long long) == 8);
    typedef unsigned long long uint64_t;

    static_assert(sizeof(int) == 4);
    typedef unsigned int uint32_t;

    static_assert(sizeof(short) == 2);
    typedef unsigned short uint16_t;

    static_assert(sizeof(char) == 1);
    typedef unsigned char uint8_t;
#endif
}
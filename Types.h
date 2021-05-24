#pragma once
#include "Compat.h"

namespace ARLib {
#ifdef WINDOWS
    #if defined(ENVIRON64)
        typedef unsigned __int64 size_t;
        typedef unsigned __int64 uintptr_t;
        typedef __int64          ptrdiff_t;
        typedef __int64          intptr_t;
    #else
        typedef unsigned int     size_t;
        typedef unsigned __int64 uintptr_t;
        typedef int              ptrdiff_t;
        typedef int              intptr_t;
    #endif
        typedef unsigned __int64 uint64_t;
        typedef unsigned __int32 uint32_t;
        typedef unsigned __int16 uint16_t;
        typedef unsigned char uint8_t;
#else
    #if defined(ENVIRON64)
        typedef unsigned long long size_t;
        typedef unsigned long long uintptr_t;
        typedef long long ptrdiff_t;
        typedef long long intptr_t;
    #else
        typedef unsigned int     size_t;
        typedef unsigned int     uintptr_t;
        typedef int              ptrdiff_t;
        typedef int              intptr_t;
    #endif
    typedef unsigned long uint64_t;
    typedef unsigned int uint32_t;
    typedef unsigned short uint16_t;
    typedef unsigned char uint8_t;
#endif
}
#pragma once
#include "Compat.hpp"
#if not defined(_VA_LIST_DEFINED) and not defined(_VA_LIST)
    #ifndef _MSC_VER
typedef __builtin_va_list va_list;
    #else
typedef char* va_list;
    #endif
#endif
#ifdef WINDOWS
extern void __cdecl __va_start(va_list*, ...);

    #ifndef va_start
        #define va_start(ap, x) ((void)(__va_start(&ap, x)))
    #endif
    #ifndef va_arg
        #define va_arg(ap, t)                                                                                          \
            (                                                                                                          \
            (sizeof(t) > sizeof(__int64) || (sizeof(t) & (sizeof(t) - 1)) != 0) ?                                      \
            **(t**)((ap += sizeof(__int64)) - sizeof(__int64)) :                                                       \
            *(t*)((ap += sizeof(__int64)) - sizeof(__int64))                                                           \
            )
    #endif
    #ifndef va_end
        #define va_end(ap) ((void)(ap = (va_list)0))
    #endif
#else
    #ifndef va_start
        #define va_start(ap, param) __builtin_va_start(ap, param)
    #endif
    #ifndef va_arg
        #define va_arg(ap, type) __builtin_va_arg(ap, type)
    #endif
    #ifndef va_end
        #define va_end(ap) __builtin_va_end(ap)
    #endif
#endif
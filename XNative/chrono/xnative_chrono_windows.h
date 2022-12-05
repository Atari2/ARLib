#pragma once
#include "../../Compat.h"
#ifdef WINDOWS
    #if not defined(CHRONO_INCLUDED__) and not defined(INCLUDED_FROM_OWN_CPP___) and                                   \
    not defined(INCLUDED_FROM_XNATIVE_THREADS__)
        #error "Don't include the XNative files directly. Use Chrono.h"
    #endif
namespace ARLib {

    #define TIME_UTC 1
struct XTimeC {    // store time with nanosecond resolution
    __int64 sec;
    long nsec;
};
int __cdecl time_get(XTimeC*, int);

long __cdecl time_diff_to_millis(const XTimeC*);
long __cdecl time_diff_to_millis2(const XTimeC*, const XTimeC*);
long long __cdecl time_get_ticks();

long long __cdecl query_perf_counter();
long long __cdecl query_perf_frequency();
}    // namespace ARLib
#endif
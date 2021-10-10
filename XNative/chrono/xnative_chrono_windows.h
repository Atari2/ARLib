#pragma once
#include "../../Compat.h"
#ifdef WINDOWS
#if not defined(CHRONO_INCLUDED__) and not defined(INCLUDED_FROM_OWN_CPP___)
#error "Don't include the XNative files directly. Use Use Chrono.h"
#endif

namespace ARLib {

    struct XTimeC { // store time with nanosecond resolution
        __int64 sec;
        long nsec;
    };

    int __cdecl time_get(XTimeC*, int);

    long __cdecl time_diff_to_millis(const XTimeC*);
    long __cdecl time_diff_to_millis2(const XTimeC*, const XTimeC*);
    long long __cdecl time_get_ticks();

    long long __cdecl query_perf_counter();
    long long __cdecl query_perf_frequency();
}
#endif
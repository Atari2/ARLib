#ifndef INCLUDED_FROM_OWN_CPP___
#define INCLUDED_FROM_OWN_CPP___
#endif
#include "xnative_chrono_windows.h"
#include "../../Conversion.h"
#ifdef WINDOWS
#include <windows.h>
#include <intrin.h>
namespace ARLib {

constexpr long long Epoch      = 0x19DB1DED53E8000LL;
constexpr long Nsec_per_sec    = 1000000000L;
constexpr long Nsec_per_msec   = 1000000L;
constexpr int Msec_per_sec     = 1000;
constexpr long Nsec100_per_sec = Nsec_per_sec / 100;
static void os_get_time(XTimeC* xt) {
    unsigned long long now = time_get_ticks();
    xt->sec                = static_cast<__time64_t>(now / Nsec100_per_sec);
    xt->nsec               = static_cast<long>(now % Nsec100_per_sec) * 100;
}
static void xtime_normalize(XTimeC* xt) {
    while (xt->nsec < 0) {
        xt->sec -= 1;
        xt->nsec += Nsec_per_sec;
    }
    while (Nsec_per_sec <= xt->nsec) {
        xt->sec += 1;
        xt->nsec -= Nsec_per_sec;
    }
}
static XTimeC xtime_diff(const XTimeC* xt, const XTimeC* now) {
    XTimeC diff = *xt;
    xtime_normalize(&diff);
    if (diff.nsec < now->nsec) {
        diff.sec -= now->sec + 1;
        diff.nsec += Nsec_per_sec - now->nsec;
    } else {    // no underflow
        diff.sec -= now->sec;
        diff.nsec -= now->nsec;
    }
    if (diff.sec < 0 || (diff.sec == 0 && diff.nsec <= 0)) {
        diff.sec  = 0;
        diff.nsec = 0;
    }
    return diff;
}
int __cdecl time_get(XTimeC* time, int flag) {
    if (flag != TIME_UTC || time == nullptr) {
        flag = 0;
    } else {
        os_get_time(time);
    }
    return flag;
}
long __cdecl time_diff_to_millis(const XTimeC* time) {
    XTimeC now{};
    time_get(&now, TIME_UTC);
    return time_diff_to_millis2(time, &now);
}
long __cdecl time_diff_to_millis2(const XTimeC* first, const XTimeC* second) {
    XTimeC diff = xtime_diff(first, second);
    return static_cast<long>(diff.sec * Msec_per_sec + (diff.nsec + Nsec_per_msec - 1) / Nsec_per_msec);
}
long long __cdecl time_get_ticks() {
    FILETIME ft;
    GetSystemTimePreciseAsFileTime(&ft);
    return ((static_cast<long long>(ft.dwHighDateTime)) << 32) + static_cast<long long>(ft.dwLowDateTime) - Epoch;
}
long long __cdecl query_perf_counter() {
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return li.QuadPart;
}
long long __cdecl query_perf_frequency() {
    alignas(sizeof(long long)) static long long freq_cached = 0;
    // memory order relaxed == 0
    long long as_bytes = __iso_volatile_load64(&freq_cached);
    long long freq     = reinterpret_cast<long long&>(as_bytes);

    if (freq == 0) {
        LARGE_INTEGER li;
        QueryPerformanceFrequency(&li);
        freq = li.QuadPart;
        __iso_volatile_store64(&freq_cached, freq);
    }
    return freq;
}
}    // namespace ARLib
#endif

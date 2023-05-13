#ifndef INCLUDED_FROM_OWN_CPP___
#define INCLUDED_FROM_OWN_CPP___
#endif
#include "XNative/chrono/xnative_chrono_merge.hpp"
#include "FileSystem.hpp"
#include "Conversion.hpp"
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
long __cdecl xtime_to_millis(const XTimeC* time) {
    return static_cast<long>(time->sec * Msec_per_sec + (time->nsec + Nsec_per_msec - 1) / Nsec_per_msec);
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
int64_t get_filetime_precise() {
    FILETIME ft{};
    GetSystemTimePreciseAsFileTime(&ft);
    int64_t merged = (static_cast<int64_t>(ft.dwHighDateTime) << 32) + static_cast<int64_t>(ft.dwLowDateTime);
    return (merged - Win32TicksFromEpoch) * 100;
}
static int64_t filetime_diff_in_ms(const FILETIME& a, const FILETIME& b) {
    ULARGE_INTEGER mergeda{
        .LowPart = a.dwLowDateTime, .HighPart = a.dwHighDateTime
    };
    ULARGE_INTEGER mergedb{
        .LowPart = b.dwLowDateTime, .HighPart = b.dwHighDateTime
    };
    ULARGE_INTEGER diff{ .QuadPart = mergedb.QuadPart - mergeda.QuadPart };
    FILETIME fd{ .dwLowDateTime = diff.LowPart, .dwHighDateTime = diff.HighPart };
    int64_t diff_100ns_intervals =
    (static_cast<int64_t>(fd.dwHighDateTime) << 32) + static_cast<int64_t>(fd.dwLowDateTime);
    return diff_100ns_intervals / 10'000;
}
void Date::fill_date(const Instant& inst, Date& d) {
    FILETIME time{};
    SYSTEMTIME stime{};
    int64_t filetime    = (inst.raw_value().value / 100LL) + Win32TicksFromEpoch;
    time.dwLowDateTime  = (filetime & MAXDWORD);
    time.dwHighDateTime = (filetime >> (sizeof(DWORD) * CHAR_BIT)) & MAXDWORD;
    if (d.has_timezone()) {
        FILETIME ltime{};
        FileTimeToLocalFileTime(&time, &ltime);
        auto utcdiff                = filetime_diff_in_ms(time, ltime) / 3'600'000;
        d.m_tz_info.m_diff_from_utc = static_cast<int8_t>(utcdiff);
        FileTimeToSystemTime(&ltime, &stime);
    } else {
        FileTimeToSystemTime(&time, &stime);
    }
    d.m_day             = static_cast<uint8_t>(stime.wDay);
    d.m_dayofweek       = static_cast<uint8_t>(stime.wDayOfWeek);
    d.m_year            = static_cast<uint16_t>(stime.wYear);
    d.m_month           = static_cast<uint8_t>(stime.wMonth);
    d.m_hour            = static_cast<uint8_t>(stime.wHour);
    d.m_minute          = static_cast<uint8_t>(stime.wMinute);
    d.m_second          = static_cast<uint8_t>(stime.wSecond);
    d.m_extra_precision = Millis{ stime.wMilliseconds };
}
Instant Date::date_to_instant(const Date& d) {
    FILETIME time{};
    SYSTEMTIME stime{};
    stime.wYear         = d.m_year;
    stime.wMonth        = d.m_month;
    stime.wDay          = d.m_day;
    stime.wDayOfWeek    = d.m_dayofweek;
    stime.wHour         = d.m_hour;
    stime.wMinute       = d.m_minute;
    stime.wSecond       = d.m_second;
    stime.wMilliseconds = static_cast<WORD>(d.m_extra_precision.millis().value);
    if (d.has_timezone()) {
        FILETIME ltime{};
        SystemTimeToFileTime(&stime, &ltime);
        LocalFileTimeToFileTime(&ltime, &time);
    } else {
        SystemTimeToFileTime(&stime, &time);
    }
    // 100-nanoseconds ticks
    int64_t raw_value = static_cast<int64_t>(time.dwLowDateTime) | (static_cast<int64_t>(time.dwHighDateTime) << 32);
    raw_value -= Win32TicksFromEpoch;
    return Instant::from_nanos(Nanos{ raw_value * 100LL });
}
String Date::date_to_string(const Date& d, Date::Format fmt) {
    char buf[256];
    if (fmt == Format::YYYYDDMMhhmmss && d.has_timezone()) {
        int ret = ARLib::sprintf(
        buf, "%04hu-%02hhu-%02hhu %02hhu:%02hhu:%02hhuUTC%+03hhd", d.m_year, d.m_month, d.m_day, d.m_hour, d.m_minute,
        d.m_second, d.m_tz_info.m_diff_from_utc
        );
        buf[ret] = '\0';
    } else if (fmt == Format::YYYYDDMMhhmmss) {
        int ret = ARLib::sprintf(
        buf, "%04hu-%02hhu-%02hhu %02hhu:%02hhu:%02hhu", d.m_year, d.m_month, d.m_day, d.m_hour, d.m_minute, d.m_second
        );
        buf[ret] = '\0';
    } else /* Format::WithEnglishNames */ {
        // Tuesday, May 2 2023, 16:13:40
        StringView weekday = d.dayname();
        StringView monthn  = d.monthname();
        int ret            = ARLib::sprintf(
        buf, "%s, %s %02hhu %04hu, %02hhu:%02hhu:%02hhu", weekday.data(), monthn.data(), d.m_day, d.m_year, d.m_hour,
        d.m_minute, d.m_second
        );
        buf[ret] = '\0';
    }
    return String{ buf };
}
}    // namespace ARLib
#endif

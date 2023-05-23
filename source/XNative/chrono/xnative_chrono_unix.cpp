#ifndef INCLUDED_FROM_OWN_CPP___
#define INCLUDED_FROM_OWN_CPP___
#endif
#include "Chrono.hpp"
#ifdef UNIX_OR_MINGW
#ifndef ON_MINGW
    #include <sys/syscall.h>
#endif
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
namespace ARLib {
TimeSpecC time_get() {
    TimeSpecC tp{};
#ifndef ON_MINGW
    syscall(SYS_clock_gettime, 0 /* CLOCK_REALTIME */, reinterpret_cast<timespec*>(&tp));
#else
    clock_gettime(0 /* CLOCK_REALTIME */, reinterpret_cast<timespec*>(&tp));
#endif
    return tp;
}
void Date::fill_date(const Instant& inst, Date& d) {
    timespec spec{};
    Nanos raw           = inst.raw_value();
    spec.tv_sec         = raw.to<Seconds>().value;
    spec.tv_nsec        = (raw - raw.to<Seconds>()).value;
    time_t time         = spec.tv_sec;
    struct tm* timeinfo = nullptr;
    if (d.has_timezone()) {
        timeinfo                    = localtime(&time);
        d.m_tz_info.m_diff_from_utc = static_cast<int8_t>(timeinfo->tm_gmtoff / 3600);
    } else {
        timeinfo = gmtime(&time);
    }
    d.m_day             = static_cast<uint8_t>(timeinfo->tm_mday);
    d.m_dayofweek       = static_cast<uint8_t>(timeinfo->tm_wday);
    d.m_year            = static_cast<uint16_t>(timeinfo->tm_year);
    d.m_month           = static_cast<uint8_t>(timeinfo->tm_mon);
    d.m_hour            = static_cast<uint8_t>(timeinfo->tm_hour);
    d.m_minute          = static_cast<uint8_t>(timeinfo->tm_min);
    d.m_second          = static_cast<uint8_t>(timeinfo->tm_sec);
    d.m_extra_precision = Nanos{ spec.tv_nsec };
}
Instant Date::date_to_instant(const Date& d) {
    struct tm timeinfo {};
    timeinfo.tm_mday = d.m_day;
    timeinfo.tm_wday = d.m_dayofweek;
    timeinfo.tm_year = d.m_year;
    timeinfo.tm_mon  = d.m_month;
    timeinfo.tm_hour = d.m_hour;
    timeinfo.tm_min  = d.m_minute;
    timeinfo.tm_sec  = d.m_second;
    timeinfo.tm_yday = d.yearday();
    time_t t         = mktime(&timeinfo);
    return Instant::from_nanos(Seconds{ t } + d.m_extra_precision.nanos());
}
String Date::date_to_string(const Date& d, Date::Format fmt) {
    char buf[256];
    uint16_t real_year = d.m_year + 1900;
    uint8_t real_month = d.m_month + 1;
    if (fmt == Format::YYYYDDMMhhmmss && d.has_timezone()) {
        int ret = ARLib::sprintf(
        buf, "%04hu-%02hhu-%02hhu %02hhu:%02hhu:%02hhuUTC%+03hhd", real_year, real_month, d.m_day, d.m_hour, d.m_minute,
        d.m_second, d.m_tz_info.m_diff_from_utc
        );
        buf[ret] = '\0';
    } else if (fmt == Format::YYYYDDMMhhmmss) {
        int ret = ARLib::sprintf(
        buf, "%04hu-%02hhu-%02hhu %02hhu:%02hhu:%02hhu", real_year, real_month, d.m_day, d.m_hour, d.m_minute,
        d.m_second
        );
        buf[ret] = '\0';
    } else /* Format::WithEnglishNames */ {
        // Tuesday, May 2 2023, 16:13:40
        StringView weekday = d.dayname();
        StringView monthn  = d.monthname();
        int ret            = ARLib::sprintf(
        buf, "%s, %s %02hhu %04hu, %02hhu:%02hhu:%02hhu", weekday.data(), monthn.data(), d.m_day, real_year, d.m_hour,
        d.m_minute, d.m_second
        );
        buf[ret] = '\0';
    }
    return String{ buf };
}
}    // namespace ARLib
#endif

#ifndef INCLUDED_FROM_OWN_CPP___
    #define INCLUDED_FROM_OWN_CPP___
#endif
#include "XNative/chrono/xnative_chrono_merge.hpp"
namespace ARLib {
Date::Date(Instant instant) {
    fill_date(instant, *this);
}
Instant Date::to_instant() const {
    return date_to_instant(*this);
}
Duration Date::diff(const Date& other) const {
    return date_to_instant(*this).raw_value() - date_to_instant(other).raw_value();
}
String Date::to_string(Date::Format fmt) const {
    return date_to_string(*this, fmt);
}
StringView Date::dayname() const {
    constexpr static StringView daynames[] = { "Sunday"_sv,   "Monday"_sv, "Tuesday"_sv, "Wednesday"_sv,
                                               "Thursday"_sv, "Friday"_sv, "Saturday"_sv };
    return daynames[m_dayofweek];
}
StringView Date::monthname() const {
    constexpr static StringView monthnames[] = { "January"_sv,   "February"_sv, "March"_sv,    "April"_sv,
                                                 "May"_sv,       "June"_sv,     "July"_sv,     "August"_sv,
                                                 "September"_sv, "October"_sv,  "November"_sv, "December"_sv };
#ifdef UNIX_OR_MINGW
    return monthnames[m_month];
#else
    return monthnames[m_month - 1];
#endif
}
uint16_t Date::yearday() const {
    constexpr static uint8_t days_in_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    bool is_leap_year                        = false;
    uint16_t daynum                          = 0;
#ifdef UNIX_OR_MINGW
    const uint16_t real_year = m_year + 1900;
    if (real_year % 4 == 0) {
        if (real_year % 100 == 0) {
            is_leap_year = (real_year % 400) == 0;
        } else {
            is_leap_year = true;
        }
    }
    for (uint8_t i = 0; i < m_month; ++i) { daynum += days_in_month[i] + (i == 1 && is_leap_year); }
#else
    if (m_year % 4 == 0) {
        if (m_year % 100 == 0) {
            is_leap_year = (m_year % 400) == 0;
        } else {
            is_leap_year = true;
        }
    }
    for (uint8_t i = 0; i < (m_month - 1); ++i) { daynum += days_in_month[i] + (i == 1 && is_leap_year); }
#endif
    return daynum + 1;    // 1st of Jan is 1st day, not 0th day
}
Instant ChronoNative::now() {
    constexpr auto den = 1'000'000'000L;
#ifdef UNIX_OR_MINGW
    auto spec = time_get();
    return Instant::from_nanos(Nanos{ (static_cast<int64_t>(spec.tv_sec) * den) + spec.tv_nsec });
#else
    constexpr long long tenmhz = 10'000'000;
    const long long Freq       = query_perf_frequency();
    const long long Ctr        = query_perf_counter();
    if (Freq == tenmhz) {
        constexpr __int64 Multiplier = den / tenmhz;
        return Instant::from_nanos(Nanos{ Ctr * Multiplier });
    } else {
        const __int64 Whole = (Ctr / Freq) * den;
        const __int64 Part  = (Ctr % Freq) * den / Freq;
        return Instant::from_nanos(Nanos{ Whole + Part });
    }
#endif
}
Instant ChronoNative::datenow() {
#ifdef UNIX_OR_MINGW
    constexpr auto den = 1'000'000'000L;
    auto spec          = time_get();
    return Instant::from_nanos(Nanos{ (static_cast<int64_t>(spec.tv_sec) * den) + spec.tv_nsec });
#else
    return Instant::from_nanos(Nanos{ get_filetime_precise() });
#endif
}
}    // namespace ARLib

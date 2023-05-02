#pragma once
#include "Types.hpp"
#include "Concepts.hpp"
#include "Pair.hpp"
#include "Ordering.hpp"
#include "PrintInfo.hpp"
#include "CharConv.hpp"
#include "Variant.hpp"
#ifdef UNIX_OR_MINGW
    #include "XNative/chrono/xnative_chrono_unix.hpp"
#else
    #include "XNative/chrono/xnative_chrono_windows.hpp"
#endif

#if not defined(CHRONO_INCLUDED__) and not defined(INCLUDED_FROM_OWN_CPP___)
    #error "Don't include the XNative files directly. Use Chrono.h"
#endif
namespace ARLib {

struct Seconds;
struct Millis;
struct Micros;
struct Nanos;

template <typename T>
concept TimeUnitType = IsAnyOfV<T, Seconds, Millis, Micros, Nanos>;
struct Seconds {
    int64_t value;

    template <TimeUnitType T>
    constexpr T to() const;
    template <TimeUnitType T>
    constexpr operator T() const {
        return to<T>();
    }
};
struct Millis {
    int64_t value;

    template <TimeUnitType T>
    constexpr T to() const;
    template <TimeUnitType T>
    constexpr operator T() const {
        return to<T>();
    }
};
struct Micros {
    int64_t value;

    template <TimeUnitType T>
    constexpr T to() const;
    template <TimeUnitType T>
    constexpr operator T() const {
        return to<T>();
    }
};
struct Nanos {
    int64_t value;

    template <TimeUnitType T>
    constexpr T to() const;
    template <TimeUnitType T>
    constexpr operator T() const {
        return to<T>();
    }
};
namespace internal {
    enum class TimeConversionAction { Non, Mul, Div };
    using TCP = Pair<TimeConversionAction, int64_t>;
    // clang-format off
    constexpr static TCP time_conversion_matrix[4][4] = {
        { { TimeConversionAction::Non, 1 },             { TimeConversionAction::Div, 1'000 },     { TimeConversionAction::Div, 1'000'000 }, { TimeConversionAction::Div, 1'000'000'000 }},
        { { TimeConversionAction::Mul, 1'000 },         { TimeConversionAction::Non, 1 },         { TimeConversionAction::Div, 1'000 },     { TimeConversionAction::Div, 1'000'000 }    },
        { { TimeConversionAction::Mul, 1'000'000 },     { TimeConversionAction::Mul, 1'000 },     { TimeConversionAction::Non, 1 },         { TimeConversionAction::Div, 1'000 }        },
        { { TimeConversionAction::Mul, 1'000'000'000 }, { TimeConversionAction::Mul, 1'000'000 }, { TimeConversionAction::Mul, 1'000 },     { TimeConversionAction::Non, 1 }            }
    };
    // clang-format on

    template <TimeUnitType To, TimeUnitType From>
    constexpr To convert_time_unit(From unit) {
        using TimeArray      = TypeArray<Seconds, Millis, Micros, Nanos>;
        constexpr size_t row = TimeArray::IndexOf<To>;
        constexpr size_t col = TimeArray::IndexOf<From>;

        constexpr auto p      = time_conversion_matrix[row][col];
        constexpr auto action = p.first();
        constexpr auto num    = p.second();
        switch (action) {
            case TimeConversionAction::Non:
                return To{ unit.value };
            case TimeConversionAction::Mul:
                return To{ unit.value * num };
            case TimeConversionAction::Div:
                return To{ unit.value / num };
        }
        unreachable;
    }
    using BetweenTimesOp = int64_t (*)(int64_t, int64_t);
    constexpr int64_t op_sum(int64_t lhs, int64_t rhs) {
        return lhs + rhs;
    }
    constexpr int64_t op_diff(int64_t lhs, int64_t rhs) {
        return lhs - rhs;
    }
    constexpr int64_t op_mult(int64_t lhs, int64_t rhs) {
        return lhs * rhs;
    }
    constexpr int64_t op_div(int64_t lhs, int64_t rhs) {
        return lhs / rhs;
    }
    enum BetweenTimesOpType { Sum, Diff, Mult, Div };
    template <BetweenTimesOpType type>
    constexpr BetweenTimesOp choose_op() {
        switch (type) {
            case BetweenTimesOpType::Sum:
                return op_sum;
            case BetweenTimesOpType::Diff:
                return op_diff;
            case BetweenTimesOpType::Mult:
                return op_mult;
            case BetweenTimesOpType::Div:
                return op_div;
        }
        unreachable;
    }
    template <TimeUnitType Lhs, TimeUnitType Rhs, BetweenTimesOpType OpType>
    constexpr auto op_time_unit(Lhs lhs, Rhs rhs) {
        using TimeArray             = TypeArray<Seconds, Millis, Micros, Nanos>;
        constexpr size_t rhs_id     = TimeArray::IndexOf<Rhs>;
        constexpr size_t lhs_id     = TimeArray::IndexOf<Lhs>;
        constexpr BetweenTimesOp op = choose_op<OpType>();
        if constexpr (rhs_id >= lhs_id) {
            Rhs lhs_conv = convert_time_unit<Rhs, Lhs>(lhs);
            return Rhs{ op(lhs_conv.value, rhs.value) };
        } else {
            Lhs rhs_conv = convert_time_unit<Lhs, Rhs>(rhs);
            return Lhs{ op(lhs.value, rhs_conv.value) };
        }
    }
    template <TimeUnitType Lhs, TimeUnitType Rhs>
    constexpr Ordering compare_time_unit(Lhs lhs, Rhs rhs) {
        using TimeArray         = TypeArray<Seconds, Millis, Micros, Nanos>;
        constexpr size_t rhs_id = TimeArray::IndexOf<Rhs>;
        constexpr size_t lhs_id = TimeArray::IndexOf<Lhs>;
        if constexpr (rhs_id >= lhs_id) {
            Rhs lhs_conv = convert_time_unit<Rhs, Lhs>(lhs);
            return CompareThreeWay(lhs_conv.value, rhs.value);
        } else {
            Lhs rhs_conv = convert_time_unit<Lhs, Rhs>(rhs);
            return CompareThreeWay(lhs.value, rhs_conv.value);
        }
    }
}    // namespace internal
template <TimeUnitType T>
constexpr T Seconds::to() const {
    return internal::convert_time_unit<T>(*this);
}
template <TimeUnitType T>
constexpr T Millis::to() const {
    return internal::convert_time_unit<T>(*this);
}
template <TimeUnitType T>
constexpr T Micros::to() const {
    return internal::convert_time_unit<T>(*this);
}
template <TimeUnitType T>
constexpr T Nanos::to() const {
    return internal::convert_time_unit<T>(*this);
}
template <TimeUnitType Lhs, TimeUnitType Rhs>
constexpr auto operator+(const Lhs& lhs, const Rhs& rhs) {
    return internal::op_time_unit<Lhs, Rhs, internal::BetweenTimesOpType::Sum>(lhs, rhs);
}
template <TimeUnitType Lhs, TimeUnitType Rhs>
constexpr auto operator-(const Lhs& lhs, const Rhs& rhs) {
    return internal::op_time_unit<Lhs, Rhs, internal::BetweenTimesOpType::Diff>(lhs, rhs);
}
template <TimeUnitType Lhs, TimeUnitType Rhs>
constexpr auto operator*(const Lhs& lhs, const Rhs& rhs) {
    return internal::op_time_unit<Lhs, Rhs, internal::BetweenTimesOpType::Mult>(lhs, rhs);
}
template <TimeUnitType Lhs, TimeUnitType Rhs>
constexpr auto operator/(const Lhs& lhs, const Rhs& rhs) {
    return internal::op_time_unit<Lhs, Rhs, internal::BetweenTimesOpType::Div>(lhs, rhs);
}
template <TimeUnitType Lhs, TimeUnitType Rhs>
constexpr Ordering operator<=>(const Lhs& lhs, const Rhs& rhs) {
    return internal::compare_time_unit(lhs, rhs);
}
constexpr Seconds operator""_sec(unsigned long long value) {
    return Seconds{ static_cast<int64_t>(value) };
};
constexpr Millis operator""_ms(unsigned long long value) {
    return Millis{ static_cast<int64_t>(value) };
};
constexpr Micros operator""_us(unsigned long long value) {
    return Micros{ static_cast<int64_t>(value) };
};
constexpr Nanos operator""_ns(unsigned long long value) {
    return Nanos{ static_cast<int64_t>(value) };
};
#define PRINT_IMPL_FOR_TIME(UnitType, ext)                                                                             \
    template <>                                                                                                        \
    struct PrintInfo<UnitType> {                                                                                       \
        const UnitType& m_unit;                                                                                        \
        String repr() const { return IntToStr(m_unit.value) + " " ext; }                                               \
    }
PRINT_IMPL_FOR_TIME(Seconds, "s");
PRINT_IMPL_FOR_TIME(Millis, "ms");
PRINT_IMPL_FOR_TIME(Micros, "us");
PRINT_IMPL_FOR_TIME(Nanos, "ns");
class CommonTime {
    using TimeArray = TypeArray<Seconds, Millis, Micros, Nanos>;
    Variant<Seconds, Millis, Micros, Nanos> m_time;
    enum class Type { Seconds, Millis, Micros, Nanos };
    Type m_type;
    template <TimeUnitType T>
    Ordering compare_self_with_other(const T& other) const;
    friend PrintInfo<CommonTime>;

    public:
    CommonTime() : m_time(Seconds{ 0 }), m_type(Type::Seconds) {}
    template <TimeUnitType T>
    constexpr CommonTime(T time) : m_time(time), m_type(to_enum<Type>(TimeArray::IndexOf<T>)) {}
    CommonTime(const CommonTime& other)                = default;
    CommonTime& operator=(const CommonTime& other)     = default;
    CommonTime(CommonTime&& other) noexcept            = default;
    CommonTime& operator=(CommonTime&& other) noexcept = default;
    auto type() const { return m_type; }
    auto nanos() const {
        Nanos ns{ 0 };
        m_time.visit([&ns](auto&& time) { ns = time; });
        return ns;
    }
    auto micros() const {
        Micros us{ 0 };
        m_time.visit([&us](auto&& time) { us = time; });
        return us;
    }
    auto millis() const {
        Millis ms{ 0 };
        m_time.visit([&ms](auto&& time) { ms = time; });
        return ms;
    }
    auto seconds() const {
        Seconds s{ 0 };
        m_time.visit([&s](auto&& time) { s = time; });
        return s;
    }
    Ordering operator<=>(const CommonTime& other) const {
        switch (other.type()) {
            case Type::Seconds:
                return compare_self_with_other(other.m_time.get<Seconds>());
            case Type::Millis:
                return compare_self_with_other(other.m_time.get<Millis>());
            case Type::Micros:
                return compare_self_with_other(other.m_time.get<Micros>());
            case Type::Nanos:
                return compare_self_with_other(other.m_time.get<Nanos>());
        }
        unreachable;
    }
};
// A nanosecond precision instant timestamp
// the raw value isn't the same across OSes.
class Instant {
    Nanos m_instant;
    Instant(Nanos ns) : m_instant{ ns } {}

    public:
    Instant(TimeUnitType auto&& inst) : m_instant{ inst.template to<Nanos>() } {}
    static Instant from_nanos(Nanos ns) { return Instant{ ns }; }
    Nanos raw_value() const { return m_instant; }
    CommonTime to_common() const { return CommonTime{ m_instant }; }
    Ordering operator<=>(const Instant& other) const { return m_instant <=> other.m_instant; }
};
class Duration {
    Nanos m_duration;

    public:
    Duration(TimeUnitType auto&& dur) : m_duration{ dur.template to<Nanos>() } {}
    Nanos raw_value() const { return m_duration; }
    CommonTime to_common() const { return CommonTime{ m_duration }; }
    Ordering operator<=>(const Duration& other) const { return m_duration <=> other.m_duration; }
};
// date with second precision
class Date {
    public:
    enum class Format { YYYYDDMMhhmmss, WithEnglishNames };

    private:
    uint16_t m_year;
    uint8_t m_month;        // 1-12 on Win32, 0-11 on Unix
    uint8_t m_dayofweek;    // 0-6
    uint8_t m_day;          // 1-31
    uint8_t m_hour;         // 0-23
    uint8_t m_minute;       // 0-59
    uint8_t m_second;       // 0-59 on Win32, 0-60 on Unix
    static void fill_date(const Instant&, Date&);
    static Instant date_to_instant(const Date&);
    // YYYY-MM-DD hh:mm:ss
    static String date_to_string(const Date&, Format);
    CommonTime m_extra_precision;    // millisecond on Win32, nanoseconds on Unix

    public:
    Date(Instant instant);
    Instant to_instant() const;
    Duration diff(const Date& other) const;
    String to_string(Format fmt = Format::YYYYDDMMhhmmss) const;
    StringView dayname() const;
    StringView monthname() const;
    uint16_t yearday() const;
    CommonTime extra_precision() const { return m_extra_precision; }
};
template <>
struct PrintInfo<Duration> {
    const Duration& m_duration;
    PrintInfo(const Duration& duration) : m_duration(duration) {}
    String repr() const { return print_conditional(m_duration.raw_value()); }
};
template <>
struct PrintInfo<Instant> {
    const Instant& m_instant;
    PrintInfo(const Instant& instant) : m_instant(instant) {}
    String repr() const { return print_conditional(m_instant.raw_value()); }
};
template <>
struct PrintInfo<Date> {
    const Date& m_date;
    PrintInfo(const Date& date) : m_date(date) {}
    String repr() const {
        return m_date.to_string();
    }
};
template <>
struct PrintInfo<CommonTime> {
    const CommonTime& m_time;
    PrintInfo(const CommonTime& time) : m_time(time) {}
    String repr() const {
        String str{};
        m_time.m_time.visit([&str](auto&& time) { str = print_conditional(time); });
        return str;
    }
};
template <TimeUnitType T>
Ordering CommonTime::compare_self_with_other(const T& other) const {
    Ordering ord = unordered;
    m_time.visit([&ord, &other](auto&& time) { ord = time <=> other; });
    return ord;
}
using TimeDiff = Nanos;
class ChronoNative {
    public:
    static Instant now();
    static Instant datenow();
};
}    // namespace ARLib

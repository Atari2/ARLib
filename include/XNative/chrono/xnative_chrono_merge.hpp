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
        String repr() const {                                                                                          \
            return IntToStr(m_unit.value) + " " ext;                                                                   \
        }                                                                                                              \
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
    static Nanos now();
};
}    // namespace ARLib

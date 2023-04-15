#pragma once
#include "Types.hpp"
#include "Concepts.hpp"
#include "Pair.hpp"
#include "Ordering.hpp"
#include "PrintInfo.hpp"
#include "CharConv.hpp"
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
    Ordering operator<=>(const Nanos& other) const { return CompareThreeWay(this->value, other.value); };
    template <TimeUnitType T>
    constexpr T to() const;
    template <TimeUnitType T>
    constexpr operator T() const {
        return to<T>();
    }
};
namespace internal {
    enum class TimeConversionAction { Non, Mul, Div };
    using enum TimeConversionAction;
    using TCP = Pair<TimeConversionAction, int64_t>;
    // clang-format off
    constexpr static TCP time_conversion_matrix[4][4] = {
        { { Non, 1 },             { Div, 1'000 },     { Div, 1'000'000 }, { Div, 1'000'000'000 }},
        { { Mul, 1'000 },         { Non, 1 },         { Div, 1'000 },     { Div, 1'000'000 }    },
        { { Mul, 1'000'000 },     { Mul, 1'000 },     { Non, 1 },         { Div, 1'000 }        },
        { { Mul, 1'000'000'000 }, { Mul, 1'000'000 }, { Mul, 1'000 },     { Non, 1 }            }
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
            case Non:
                return To{ unit.value };
            case Mul:
                return To{ unit.value * num };
            case Div:
                return To{ unit.value / num };
        }
        unreachable;
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
#define PRINT_IMPL_FOR_TIME(UnitType)                                                                                  \
    template <>                                                                                                        \
    struct PrintInfo<UnitType> {                                                                                       \
        const UnitType& m_unit;                                                                                        \
        String repr() const { return IntToStr(m_unit.value); }                                                         \
    }
PRINT_IMPL_FOR_TIME(Seconds);
PRINT_IMPL_FOR_TIME(Millis);
PRINT_IMPL_FOR_TIME(Micros);
PRINT_IMPL_FOR_TIME(Nanos);
using TimeDiff = Nanos;
class ChronoNative {
    public:
    static Nanos now();
};
}    // namespace ARLib

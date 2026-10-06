#pragma once

#include "Compat.hpp"
#include "Concepts.hpp"
#include "Types.hpp"
namespace ARLib {
#ifdef COMPILER_CLANG
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wsign-conversion"
    #pragma clang diagnostic ignored "-Wconstant-conversion"
    #pragma clang diagnostic ignored "-Wimplicitly-unsigned-literal"
#elif COMPILER_MSVC
    #pragma warning(push)
    #pragma warning(disable : 4146)    // unary minus operator applied to unsigned type, result still unsigned
    #pragma warning(disable : 4309)    // truncation of constant value
#endif
namespace detail {
    consteval size_t StrLenFromIntegralInternal(Integral auto v) noexcept {
        constexpr bool is_signed = IsSigned<decltype(v)>;
        constexpr size_t base    = 10;
        constexpr size_t b2      = base * base;
        constexpr size_t b3      = b2 * base;
        constexpr size_t b4      = b3 * base;
        size_t value             = is_signed && v < 0 ? static_cast<size_t>(-v) : static_cast<size_t>(v);
        size_t n                 = 1 + (is_signed && v < 0);    // add +1 if negative for the minus
        for (;;) {
            if (value < base) return n;
            if (value < b2) return n + 1;
            if (value < b3) return n + 2;
            if (value < b4) return n + 3;
            value /= b4;
            n += 4;
        }
    }
}    // namespace detail
template <typename T>
struct NumberTraits {};
template <typename Traits, Integral T>
struct IntegralBaseTraits {
    constexpr static inline bool is_signed    = IsSigned<T>;
    constexpr static inline auto size         = sizeof(T);
    constexpr static inline size_t max_digits = detail::StrLenFromIntegralInternal(Traits::max);
};
template <>
struct NumberTraits<bool> : IntegralBaseTraits<NumberTraits<bool>, bool> {
    constexpr static inline bool min = false;
    constexpr static inline bool max = true;
};
template <>
struct NumberTraits<char> : IntegralBaseTraits<NumberTraits<char>, char> {
    constexpr static inline char min = -128;
    constexpr static inline char max = 127;
};
template <>
struct NumberTraits<unsigned char> : IntegralBaseTraits<NumberTraits<unsigned char>, unsigned char> {
    constexpr static inline unsigned char min = 0;
    constexpr static inline unsigned char max = 255;
};
template <>
struct NumberTraits<wchar_t> : IntegralBaseTraits<NumberTraits<wchar_t>, wchar_t> {
    constexpr static inline wchar_t min = windows_build ? 0 : -2147483647;
    constexpr static inline wchar_t max = windows_build ? 65535 : 2147483647;
};
template <>
struct NumberTraits<char16_t> : IntegralBaseTraits<NumberTraits<char16_t>, char16_t> {
    constexpr static inline char16_t min = 0;
    constexpr static inline char16_t max = 65535;
};
template <>
struct NumberTraits<char32_t> : IntegralBaseTraits<NumberTraits<char32_t>, char32_t> {
    constexpr static inline char32_t min = 0;
    constexpr static inline char32_t max = 4294967295;
};
template <>
struct NumberTraits<short> : IntegralBaseTraits<NumberTraits<short>, short> {
    constexpr static inline short min = -32768;
    constexpr static inline short max = 32767;
};
template <>
struct NumberTraits<unsigned short> : IntegralBaseTraits<NumberTraits<unsigned short>, unsigned short> {
    constexpr static inline unsigned short min = 0;
    constexpr static inline unsigned short max = 65535;
};
template <>
struct NumberTraits<int> : IntegralBaseTraits<NumberTraits<int>, int> {
    constexpr static inline int min = -2147483648;
    constexpr static inline int max = 2147483647;
};
template <>
struct NumberTraits<unsigned int> : IntegralBaseTraits<NumberTraits<unsigned int>, unsigned int> {
    constexpr static inline unsigned int min = 0;
    constexpr static inline unsigned int max = 4294967295;
};
template <>
struct NumberTraits<long> : IntegralBaseTraits<NumberTraits<long>, long> {
    constexpr static inline long min = windows_build ? -2147483648l : -9223372036854775807ll - 1ll;
    constexpr static inline long max = windows_build ? 2147483647l : 9223372036854775807l;
};
template <>
struct NumberTraits<unsigned long> : IntegralBaseTraits<NumberTraits<unsigned long>, unsigned long> {
    constexpr static inline unsigned long min = 0;
    constexpr static inline unsigned long max = windows_build ? 4294967295ul : 18446744073709551615ul;
};
template <>
struct NumberTraits<long long> : IntegralBaseTraits<NumberTraits<long long>, long long> {
    constexpr static inline long long min = -9223372036854775807ll - 1ll;
    constexpr static inline long long max = 9223372036854775807ll;
};
template <>
struct NumberTraits<unsigned long long> : IntegralBaseTraits<NumberTraits<unsigned long long>, unsigned long long> {
    constexpr static inline unsigned long long min = 0;
    constexpr static inline unsigned long long max = 18446744073709551615ull;
};
template <>
struct NumberTraits<float> {
    constexpr static inline bool is_signed  = true;
    constexpr static inline float min       = 1.175494351e-38F;
    constexpr static inline float max       = 3.402823466e+38F;
    constexpr static inline auto size       = sizeof(float);
    constexpr static inline auto max_digits = 39;    // len(str(int(3.402823466e+38))) in python
};
template <>
struct NumberTraits<double> {
    constexpr static inline bool is_signed  = true;
    constexpr static inline double min      = 2.2250738585072014e-308;
    constexpr static inline double max      = 1.7976931348623158e+308;
    constexpr static inline auto size       = sizeof(double);
    constexpr static inline auto max_digits = 309;    // len(str(int(1.7976931348623158e+308))) in python
};
template <>
struct NumberTraits<long double> {
    constexpr static inline bool is_signed = true;
#ifdef _WIN64
    constexpr static inline long double min = NumberTraits<double>::min;
    constexpr static inline long double max = NumberTraits<double>::max;
#else
    constexpr static inline long double min = 3.3621e-4932L;
    constexpr static inline long double max = 1.18973e+4932L;
#endif
    constexpr static inline auto size = sizeof(long double);
};
template <typename T>
requires Integral<T> || FloatingPoint<T>
constexpr T max([[maybe_unused]] T val) {
    return NumberTraits<RemoveCvRefT<T>>::max;
}
template <typename T>
requires Integral<T> || FloatingPoint<T>
constexpr T min([[maybe_unused]] T val) {
    return NumberTraits<RemoveCvRefT<T>>::min;
}
#ifdef COMPILER_CLANG
    #pragma clang diagnostic pop
#elif COMPILER_MSVC
    #pragma warning(pop)
#endif
}    // namespace ARLib

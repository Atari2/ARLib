#pragma once

#include "Compat.h"
#include "Concepts.h"
#include "Types.h"

namespace ARLib {
    template <typename T>
    struct NumberTraits {};

    template <>
    struct NumberTraits<bool> {
        constexpr static inline bool is_signed = false;
        constexpr static inline bool min = false;
        constexpr static inline bool max = true;
        constexpr static inline auto size = sizeof(bool);
    };

    template <>
    struct NumberTraits<char> {
        constexpr static inline bool is_signed = true;
        constexpr static inline char min = -128;
        constexpr static inline char max = 127;
        constexpr static inline auto size = sizeof(char);
    };

    template <>
    struct NumberTraits<unsigned char> {
        constexpr static inline bool is_signed = false;
        constexpr static inline unsigned char min = 0;
        constexpr static inline unsigned char max = 255;
        constexpr static inline auto size = sizeof(unsigned char);
    };

    template <>
    struct NumberTraits<wchar_t> {
        constexpr static inline bool is_signed = !windows_build;
        constexpr static inline wchar_t min = windows_build ? 0 : -2147483647;
        constexpr static inline wchar_t max = windows_build ? 65535 : 2147483647;
        constexpr static inline auto size = sizeof(wchar_t);
    };

    template <>
    struct NumberTraits<char16_t> {
        constexpr static inline bool is_signed = false;
        constexpr static inline char16_t min = 0;
        constexpr static inline char16_t max = 65535;
        constexpr static inline auto size = sizeof(char16_t);
    };

    template <>
    struct NumberTraits<char32_t> {
        constexpr static inline bool is_signed = false;
        constexpr static inline char32_t min = 0;
        constexpr static inline char32_t max = 4294967295;
        constexpr static inline auto size = sizeof(char32_t);
    };

    template <>
    struct NumberTraits<short> {
        constexpr static inline bool is_signed = true;
        constexpr static inline short min = -32768;
        constexpr static inline short max = 32767;
        constexpr static inline auto size = sizeof(short);
    };

    template <>
    struct NumberTraits<unsigned short> {
        constexpr static inline bool is_signed = false;
        constexpr static inline unsigned short min = 0;
        constexpr static inline unsigned short max = 65535;
        constexpr static inline auto size = sizeof(unsigned short);
    };

    template <>
    struct NumberTraits<int> {
        constexpr static inline bool is_signed = true;
        constexpr static inline int min = -2147483648;
        constexpr static inline int max = 2147483647;
        constexpr static inline auto size = sizeof(int);
    };

    template <>
    struct NumberTraits<unsigned int> {
        constexpr static inline bool is_signed = false;
        constexpr static inline unsigned int min = 0;
        constexpr static inline unsigned int max = 4294967295;
        constexpr static inline auto size = sizeof(unsigned int);
    };

    template <>
    struct NumberTraits<long> {
        constexpr static inline bool is_signed = true;
        constexpr static inline long min = windows_build ? -2147483648l : -9223372036854775808l;
        constexpr static inline long max = windows_build ? 2147483647l : 9223372036854775807l;
        constexpr static inline auto size = sizeof(long);
    };

    template <>
    struct NumberTraits<unsigned long> {
        constexpr static inline bool is_signed = false;
        constexpr static inline unsigned long min = 0;
        constexpr static inline unsigned long max = windows_build ? 4294967295ul : 18446744073709551615ul;
        constexpr static inline auto size = sizeof(unsigned long);
    };

    template <>
    struct NumberTraits<long long> {
        constexpr static inline bool is_signed = true;
        constexpr static inline long long min = -9223372036854775808ll;
        constexpr static inline long long max = 9223372036854775807ll;
        constexpr static inline auto size = sizeof(long long);
    };

    template <>
    struct NumberTraits<unsigned long long> {
        constexpr static inline bool is_signed = false;
        constexpr static inline unsigned long long min = 0;
        constexpr static inline unsigned long long max = 18446744073709551615ull;
        constexpr static inline auto size = sizeof(unsigned long long);
    };

    template <>
    struct NumberTraits<float> {
        constexpr static inline bool is_signed = true;
        constexpr static inline float min = 1.175494351e-38F;
        constexpr static inline float max = 3.402823466e+38F;
        constexpr static inline auto size = sizeof(float);
    };

    template <>
    struct NumberTraits<double> {
        constexpr static inline bool is_signed = true;
        constexpr static inline double min = 2.2250738585072014e-308;
        constexpr static inline double max = 1.7976931348623158e+308;
        constexpr static inline auto size = sizeof(double);
    };

    template <>
    struct NumberTraits<long double> {
        constexpr static inline bool is_signed = true;
#ifdef _WIN64
        constexpr static inline long double max = NumberTraits<double>::min;
        constexpr static inline long double max = NumberTraits<double>::max;
#else
        constexpr static inline long double min = 3.3621e-4932L;
        constexpr static inline long double max = 1.18973e+4932L;
#endif
        constexpr static inline auto size = sizeof(long double);
    };

} // namespace ARLib
#pragma once
#include "Assertion.h"
#include "Compat.h"
#include "Concepts.h"
#include "CpuInfo.h"
#include <immintrin.h>

namespace ARLib {

    consteval char operator""_c(const unsigned long long num) {
        CONSTEVAL_STATIC_ASSERT(num < 255, "Can't convert numbers over 255 to char");
        return static_cast<char>(num);
    }

    constexpr bool haszerobyte(Integral auto v) {
        if constexpr (sizeof(v) == 1)
            return !v;
        else if constexpr (sizeof(v) == 2)
            return (((v)-0x0101) & ~(v)&0x8080);
        else if constexpr (sizeof(v) == 4)
            return (((v)-0x01010101) & ~(v)&0x80808080);
        else if constexpr (sizeof(v) == 8)
            return (((v)-0x0101010101010101) & ~(v)&0x8080808080808080);
    }

    constexpr size_t strlen(const char* ptr) {
        size_t len = 0;
        while (*(ptr++))
            len++;
        return len;
    }

    template <size_t NUM>
    char* strncpy_vectorized(char* dest, const char* src) {
        static_assert(NUM % 32 == 0, "Size of strncpy vectorized must be a multiple of 32");
        for (size_t base = 0; base < NUM; base += 32) {
#ifdef COMPILER_MSVC
            __m256i buffer = _mm256_loadu_si256((const __m256i*)(src + base));
            _mm256_storeu_si256((__m256i*)(dest + base), buffer);
#else
            __m256i buffer = _mm256_loadu_si256(reinterpret_cast<const __m256i_u*>(src + base));
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(dest + base), buffer);
#endif
        }
        return dest;
    }

    constexpr char* strcpy(char* dest, const char* src) {
        while ((*dest++ = *src++))
            ;
        return dest;
    }

    constexpr char* strncpy(char* dest, const char* src, size_t num) {
        if (num == 0) { return dest; }
        while (num--) {
            *dest++ = *src++;
            if (*src == '\0') break;
            if (num == 0) break;
        }
        if (num == 0) return dest;
        while (num--) {
            *dest++ = '\0';
        }
        return dest;
    }
    constexpr int strcmp(const char* first, const char* second) {
        for (; *first == *second && *first; first++, second++)
            ;
        return *first - *second;
    }
    constexpr int strncmp(const char* first, const char* second, size_t num) {
        if (num == 0) return 0;
        for (; *first == *second && --num; first++, second++)
            if (*first == '\0') return 0;
        return *first - *second;
    }

    constexpr char* strcat_eff(char* end_of_dst, const char* src) {
        while (*src) {
            *end_of_dst++ = *src++;
        }
        *end_of_dst = *src;
        return end_of_dst;
    }

    constexpr char* strcat(char* dst, const char* src) {
        if (*dst != '\0')
            while (*++dst)
                ;
        while (*src) {
            *dst++ = *src++;
        }
        *dst = *src;
        return dst;
    }

    constexpr bool isspace(const char c) {
        return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
    }

    constexpr bool isdigit(const char c) { return c >= 48_c && c <= 57_c; }

    constexpr bool isalnum(const char c) { return isdigit(c) || (c >= 65_c && c <= 90_c) || (c >= 97_c && c <= 122_c); }

    template <size_t N>
    [[nodiscard]] constexpr bool constexpr_equality(const char (&arr1)[N], const char (&arr2)[N]) noexcept {
        return __builtin_memcmp(arr1, arr2, N) == 0;
    }

    void* memcpy_vectorized(void* dst0, const void* src0, size_t num);
    int memcmp(void* dst, const void* src, size_t num);
#if HAS_BUILTIN(__builtin_memmove)
    constexpr void* memmove(void* dst, const void* src, size_t num) { return __builtin_memmove(dst, src, num); }
#else
    void* memmove(void* dst, const void* src, size_t num);
#endif
    constexpr void* memcpy(void* dst0, const void* src0, size_t num) {
#if HAS_BUILTIN(__builtin_memcpy)
        return __builtin_memcpy(dst0, src0, num);
#else
        if (is_constant_evaluated()) {
            auto* dst = static_cast<uint8_t*>(dst0);
            const auto* src = static_cast<const uint8_t*>(src0);
            for (size_t i = 0; i < num; i++)
                dst[i] = src[i];
            return dst;
        } else {
            [[unlikely]] if (num >= 64 && cpuinfo.avx2()) { // check avx2 support
                return memcpy_vectorized(dst0, src0, num);
            }
            auto* dst = static_cast<uint8_t*>(dst0);
            const auto* src = static_cast<const uint8_t*>(src0);
            while (num--) {
                *dst++ = *src++;
            }
            return dst;
        }
#endif
    }
    void* memset(void* ptr, uint8_t value, size_t size);
    constexpr char toupper(char c) {
        if (c >= 96_c && c <= 122_c) return c - 32_c;
        return c;
    }
    constexpr char tolower(char c) {
        if (c >= 65_c && c <= 90_c) return c + 32_c;
        return c;
    }
} // namespace ARLib

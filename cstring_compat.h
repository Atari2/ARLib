#pragma once
#include "Compat.h"
#include "Concepts.h"
#include "CpuInfo.h"
#include <immintrin.h>

namespace ARLib {

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
        if (num == 0) return true;
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

    constexpr int isspace(int c) { return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r'; }

    constexpr int isdigit(int c) { return c >= 48 && c <= 57; }

    constexpr int isalnum(int c) { return isdigit(c) || (c >= 65 && c <= 90) || (c >= 97 && c <= 122); }

    template <size_t N>
    [[nodiscard]] constexpr bool constexpr_equality(const char (&arr1)[N], const char (&arr2)[N]) noexcept {
        return __builtin_memcmp(arr1, arr2, N) == 0;
    }

    void* memcpy_vectorized(void* dst0, const void* src0, size_t num);
    int memcmp(void* dst, const void* src, size_t num);
    void* memmove(void* dst, const void* src, size_t num);
    constexpr void* memcpy(void* dst0, const void* src0, size_t num) {
        if (is_constant_evaluated()) {
            uint8_t* dst = static_cast<uint8_t*>(dst0);
            const uint8_t* src = static_cast<const uint8_t*>(src0);
            for (size_t i = 0; i < num; i++)
                dst[i] = src[i];
            return dst;
        } else {
            [[unlikely]] if (num >= 64 && cpuinfo.avx2()) { // check avx2 support
                return memcpy_vectorized(dst0, src0, num);
            }
            uint8_t* dst = static_cast<uint8_t*>(dst0);
            const uint8_t* src = static_cast<const uint8_t*>(src0);
            while (num--) {
                *dst++ = *src++;
            }
            return dst;
        }
    }
    void* memset(void* ptr, uint8_t value, size_t size);
    constexpr char toupper(char c) {
        if (c >= 96 && c <= 122) return c - 32;
        return c;
    }
    constexpr char tolower(char c) {
        if (c >= 65 && c <= 90) return c + 32;
        return c;
    }
} // namespace ARLib

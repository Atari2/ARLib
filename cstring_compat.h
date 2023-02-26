#pragma once
#include "Assertion.h"
#include "Compat.h"
#include "Concepts.h"
#include "CpuInfo.h"
namespace ARLib {
consteval char operator""_c(const unsigned long long num) {
    CONSTEVAL_STATIC_ASSERT(num < 255, "Can't convert numbers over 255 to char");
    return static_cast<char>(num);
}
consteval wchar_t operator""_wc(const unsigned long long num) {
    return static_cast<wchar_t>(num);
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
size_t strlen_vectorized(const char* ptr);
constexpr size_t strlen(const char* ptr) {
#ifndef DEBUG
    if (!is_constant_evaluated()) {
        return strlen_vectorized(ptr);
    } else {
        if (!ptr) return 0;
        size_t len = 0;
        while (*(ptr++)) len++;
        return len;
    }
#else
    if (!ptr) return 0;
    size_t len = 0;
    while (*(ptr++)) len++;
    return len;
#endif
}
constexpr size_t wstrlen(const wchar_t* ptr) {
    if (!ptr) return 0;
    size_t len = 0;
    while (*(ptr++)) len++;
    return len;
}
constexpr char* strcpy(char* dest, const char* src) {
    while ((*dest++ = *src++))
        ;
    return dest;
}
constexpr int strcmp(const char* first, const char* second) {
    for (; *first == *second && *first; first++, second++)
        ;
    return *first - *second;
}
constexpr int wstrcmp(const wchar_t* first, const wchar_t* second) {
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
constexpr int wstrncmp(const wchar_t* first, const wchar_t* second, size_t num) {
    if (num == 0) return 0;
    for (; *first == *second && --num; first++, second++)
        if (*first == L'\0') return 0;
    return *first - *second;
}
constexpr char* strcat_eff(char* end_of_dst, const char* src) {
    while (*src) { *end_of_dst++ = *src++; }
    *end_of_dst = *src;
    return end_of_dst;
}
constexpr wchar_t* wstrcat_eff(wchar_t* end_of_dst, const wchar_t* src) {
    while (*src) { *end_of_dst++ = *src++; }
    *end_of_dst = *src;
    return end_of_dst;
}
constexpr char* strcat(char* dst, const char* src) {
    if (*dst != '\0')
        while (*++dst)
            ;
    while (*src) { *dst++ = *src++; }
    *dst = *src;
    return dst;
}
constexpr bool isspace(const char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}
constexpr bool wisspace(const wchar_t c) {
    return c == L' ' || c == L'\t' || c == L'\n' || c == L'\v' || c == L'\f' || c == L'\r';
}
constexpr const char* strstr(const char* str, const char* needle) {
    if (str == nullptr || needle == nullptr) return nullptr;
    size_t len        = strlen(str);
    size_t needle_len = strlen(needle);
    if (needle_len == 0) return str;
    if (needle_len > len) return nullptr;
    if (needle_len == len) { return strcmp(str, needle) == 0 ? str : nullptr; }
    size_t idx = 0;
    while (idx < len) {
        bool found = strncmp(str + idx, needle, needle_len) == 0;
        if (found)
            return str + idx;
        else
            idx++;
    }
    return nullptr;
}
constexpr bool isdigit(const char c) {
    return c >= 48_c && c <= 57_c;
}
constexpr bool isalnum(const char c) {
    return isdigit(c) || (c >= 65_c && c <= 90_c) || (c >= 97_c && c <= 122_c);
}
template <size_t N>
[[nodiscard]] constexpr bool constexpr_equality(const char (&arr1)[N], const char (&arr2)[N]) noexcept {
    return __builtin_memcmp(arr1, arr2, N) == 0;
}
void* memcpy_vectorized(void* dst0, const void* src0, size_t num);
int memcmp(const void* dst, const void* src, size_t num);
#if HAS_BUILTIN(__builtin_memmove)
constexpr void* memmove(void* dst, const void* src, size_t num) {
    return __builtin_memmove(dst, src, num);
}
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
        for (size_t i = 0; i < num; i++) dst[i] = src[i];
        return dst;
    } else {
        [[unlikely]] if (num >= 64 && cpuinfo.avx2()) {    // check avx2 support
            return memcpy_vectorized(dst0, src0, num);
        }
        auto* dst = static_cast<uint8_t*>(dst0);
        const auto* src = static_cast<const uint8_t*>(src0);
        while (num--) { *dst++ = *src++; }
        return dst;
    }
#endif
}
constexpr char* strncpy(char* dest, const char* src, size_t num) {
    memcpy(dest, src, num);
    return dest;
}
extern "C"
{
    void* arlib_memset(void* ptr, int value, size_t size);
}
inline void* memset(void* ptr, uint8_t value, size_t size) {
    return arlib_memset(ptr, value, size);
}
constexpr char toupper(char c) {
    if (c >= 96_c && c <= 122_c) return c - 32_c;
    return c;
}
constexpr char tolower(char c) {
    if (c >= 65_c && c <= 90_c) return c + 32_c;
    return c;
}
constexpr wchar_t wtoupper(wchar_t c) {
    if (c >= 96_wc && c <= 122_wc) return c - 32_wc;
    return c;
}
constexpr wchar_t wtolower(wchar_t c) {
    if (c >= 65_wc && c <= 90_wc) return c + 32_wc;
    return c;
}
}    // namespace ARLib

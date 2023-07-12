#pragma once

#include "String.hpp"
#include "TypeTraits.hpp"
#include "cstdio_compat.hpp"
#include "cstring_compat.hpp"
#include "CharConv.hpp"
namespace ARLib {
struct Console {
    static String getline(const char delim = '\n') {
        String buf{};
        char c = static_cast<char>(ARLib::getchar());
        while (c != delim && c != EOF) {
            buf += c;
            c = static_cast<char>(ARLib::getchar());
        }
        if (buf.size() == 0) return buf;
        if (buf.back() == '\r') { buf.set_size(buf.size() - 1); }
        return buf;
    }
    template <typename Type>
    requires(Integral<Type> || FloatingPoint<Type>)
    static size_t get_max_len_for_type(int base) {
        using NTArray =
        TypeArray<int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t, float, double>;
        constexpr size_t base_10_index = 0;
        constexpr size_t base_2_index  = 1;
        constexpr size_t base_8_index  = 2;
        constexpr size_t base_16_index = 3;

        constexpr size_t typeindex = NTArray::IndexOf<Type>;

        constexpr size_t SSOCAP = /* String::SMALL_STRING_CAP */ 15;

        constexpr size_t size_matrix[4][10] = {
            { 3, 3, 5, 5, 10, 10, 19, 20, 46, 316 }, // base 10
            { 8, 8, 16, 16, 32, 32, 64, 64, 32, 64 }, // base 2
            { 3, 3, 5, 6, 11, 11, 21, 22, SSOCAP, SSOCAP }, // base 8
            { 2, 2, 4, 4, 8, 8, 16, 16, SSOCAP, SSOCAP }  // base 16
        };

        switch (base) {
            case 10:
                return size_matrix[base_10_index][typeindex];
            case 2:
                return size_matrix[base_2_index][typeindex];
            case 8:
                return size_matrix[base_8_index][typeindex];
            case 16:
                return size_matrix[base_16_index][typeindex];
            default:
                return SSOCAP;
        }
    }
    template <typename Type>
    requires(Integral<Type> || FloatingPoint<Type>)
    static Result<Type> getnumber(int base = 10) {
        String buf{};
        buf.reserve(get_max_len_for_type<Type>(base));
        char c = static_cast<char>(ARLib::getchar());
        while (!isspace(c) && c != EOF) {
            buf += c;
            c = static_cast<char>(ARLib::getchar());
        }
        if (buf.size() == 0) return static_cast<Type>(0);
        if constexpr (UnsignedIntegral<Type>) {
            TRY_SET(t, StrToU64(buf, base));
            return static_cast<Type>(t);
        } else if constexpr (SignedIntegral<Type>) {
            TRY_SET(t, StrToI64(buf, base));
            return static_cast<Type>(t);
        } else /* (FloatingPoint<Type>) */ {
            TRY_SET(t, StrToDouble(buf));
            return static_cast<Type>(t);
        }
    }
    template <size_t N, typename... Args>
    static void print(const char (&fmt)[N], const Args&... args) {
        if constexpr (sizeof...(args) == 0) {
            ARLib::puts(fmt);
        } else {
            ARLib::printf(fmt, args...);
        }
    }
    template <typename T, typename... Args, typename = EnableIfT<IsAnyOfV<T, const char*, char*>>>
    static void print(T fmt, Args... args) {
        if constexpr (sizeof...(args) == 0) {
            ARLib::puts(fmt);
        } else {
            ARLib::printf(fmt, args...);
        }
    }
    template <size_t N, typename... Args>
    static void println(const char (&fmt)[N], const Args&... args) {
        if constexpr (sizeof...(args) == 0) {
            ARLib::puts(fmt);
        } else {
            ARLib::printf(fmt, args...);
        }
        ARLib::puts("\n");
    }
    template <typename T, typename... Args, typename = EnableIfT<IsAnyOfV<T, const char*, char*>>>
    static void println(T fmt, Args... args) {
        if constexpr (sizeof...(args) == 0) {
            ARLib::puts(fmt);
        } else {
            ARLib::printf(fmt, args...);
        }
        ARLib::puts("\n");
    }
};
}    // namespace ARLib

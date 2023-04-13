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
    static int getint(int base = 10) {
        String buf{};
        char c = static_cast<char>(ARLib::getchar());
        while (!isspace(c) && c != EOF) {
            buf += c;
            c = static_cast<char>(ARLib::getchar());
        }
        if (buf.size() == 0) return 0;
        return StrToInt(buf, base);
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

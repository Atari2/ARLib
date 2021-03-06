#pragma once

#include "String.h"
#include "TypeTraits.h"
#include "cstdio_compat.h"
#include "cstring_compat.h"

namespace ARLib {
    struct Console {
        static String getline(const char delim = '\n') {
            String buf{};
            char c = static_cast<char>(ARLib::getchar());
            while (c != delim && c != EOF) {
                buf += c;
                c = static_cast<char>(ARLib::getchar());
            }
            return buf;
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
} // namespace ARLib
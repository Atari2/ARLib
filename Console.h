#pragma once

#include "TypeTraits.h"
#include "cstdio_compat.h"

namespace ARLib {
    struct Console {
        template <size_t N, typename... Args>
        static void print(const char(&fmt)[N], const Args&... args) {
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

    };
}
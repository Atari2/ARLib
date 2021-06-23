#pragma once
#include "Compat.h"
#include "Macros.h"
#include "cstdio_compat.h"

void abort_arlib();
void assertion_failed__(const char* msg);

#define HARD_ASSERT(val, msg)                                                                                          \
    if (!val) {                                                                                                        \
        ARLib::puts(msg);                                                                                              \
        assertion_failed__(ERRINFO);                                                                                   \
        unreachable                                                                                                    \
    }
#define HARD_ASSERT_FMT(val, fmt, ...)                                                                                 \
    if (!val) {                                                                                                        \
        ARLib::printf(fmt "\n", __VA_ARGS__);                                                                          \
        assertion_failed__(ERRINFO);                                                                                   \
        unreachable                                                                                                    \
    }

#define SOFT_ASSERT(val, msg)                                                                                          \
    if (!val) {                                                                                                        \
        ARLib::puts(msg);                                                                                              \
        ARLib::puts(ERRINFO);                                                                                          \
    }
#define SOFT_ASSERT_FMT(val, fmt, ...)                                                                                 \
    if (!val) {                                                                                                        \
        ARLib::printf(fmt "\n", __VA_ARGS__);                                                                          \
        ARLib::puts(ERRINFO);                                                                                          \
    }

#define TODO_CLS(cls)                                                                                                  \
    cls() { todo__(); };                                                                                               \
    static void todo__() { HARD_ASSERT(false, CONCAT(STRINGIFY(cls), " is not implemented yet")) }
#define TODO(func) HARD_ASSERT(false, CONCAT(STRINGIFY(func), " not implemented yet"))

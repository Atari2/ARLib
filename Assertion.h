#pragma once
#include "Compat.h"
#include "Macros.h"
#include "SourceLocation.h"
#include "TypeTraits.h"
#include "cstdio_compat.h"

void abort_arlib();
void assertion_failed__();

namespace ARLib {
    enum class AssertWhat { Eq, NonEq, PtrEq, PtrNonEq };

    template <typename T1, typename T2, AssertWhat T3>
    bool assert_test(const T1& first, const T2& second) {
        if constexpr (T3 == AssertWhat::Eq) {
            return first == second;
        } else if constexpr (T3 == AssertWhat::NonEq) {
            return first != second;
        } else if constexpr (T3 == AssertWhat::PtrEq) {
            return &first == &second;
        } else if constexpr (T3 == AssertWhat::PtrNonEq) {
            return &first != &second;
        } else {
            COMPTIME_ASSERT("Invalid Assert Test")
        }
    }

    template <typename T1, typename T2>
    bool assert_eq(const T1& first, const T2& second) {
        return assert_test<T1, T2, AssertWhat::Eq>(first, second);
    }

    template <typename T1, typename T2>
    bool assert_non_eq(const T1& first, const T2& second) {
        return assert_test<T1, T2, AssertWhat::NonEq>(first, second);
    }

    template <typename T1, typename T2>
    bool assert_ptr_eq(const T1& first, const T2& second) {
        return assert_test<T1, T2, AssertWhat::PtrEq>(first, second);
    }

    template <typename T1, typename T2>
    bool assert_ptr_non_eq(const T1& first, const T2& second) {
        return assert_test<T1, T2, AssertWhat::PtrNonEq>(first, second);
    }
} // namespace ARLib

#define HARD_ASSERT(val, msg)                                                                                          \
    if (!(val)) {                                                                                                      \
        ARLib::puts("ASSERTION \"" STRINGIFY(val) "\" FAILED: " msg);                                                  \
        PRINT_SOURCE_LOCATION                                                                                          \
        assertion_failed__();                                                                                          \
        unreachable                                                                                                    \
    }
#define HARD_ASSERT_FMT(val, fmt, ...)                                                                                 \
    if (!(val)) {                                                                                                      \
        ARLib::printf("ASSERTION \"" STRINGIFY(val) "\" FAILED: " fmt "\n", __VA_ARGS__);                              \
        PRINT_SOURCE_LOCATION                                                                                          \
        assertion_failed__();                                                                                          \
        unreachable                                                                                                    \
    }

#define ASSERT_NOT_REACHED(msg) HARD_ASSERT(false, msg)

#define ASSERT_NOT_REACHED_FMT(fmt, ...) HARD_ASSERT_FMT(false, fmt, __VA_ARGS__)

#define SOFT_ASSERT(val, msg)                                                                                          \
    if (!(val)) {                                                                                                      \
        ARLib::puts("ASSERTION \"" STRINGIFY(val) "\" FAILED: " msg);                                                  \
        PRINT_SOURCE_LOCATION                                                                                          \
    }
#define SOFT_ASSERT_FMT(val, fmt, ...)                                                                                 \
    if (!(val)) {                                                                                                      \
        ARLib::printf("ASSERTION \"" STRINGIFY(val) "\" FAILED: " fmt "\n", __VA_ARGS__);                              \
        PRINT_SOURCE_LOCATION                                                                                          \
    }

#define TODO_CLS(cls)                                                                                                  \
    cls() { todo__(); };                                                                                               \
    static void todo__() { HARD_ASSERT(false, CONCAT(STRINGIFY(cls), " is not implemented yet")) }
#define TODO(func) HARD_ASSERT(false, CONCAT(STRINGIFY(func), " not implemented yet"))

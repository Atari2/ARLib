#pragma once
#include "Compat.h"
#include "Macros.h"
#include "cstdio_compat.h"
#include "TypeTraits.h"

void abort_arlib();
void assertion_failed__(const char* msg);

namespace ARLib {
    enum class AssertWhat {
        Eq,
        NonEq,
        PtrEq,
        PtrNonEq
    };

    template <typename T1, typename T2, AssertWhat T3, typename = EnableIfT<ConvertibleV<T1, T2>>>
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

    template <typename T1, typename T2, typename = EnableIfT<ConvertibleV<T1, T2>>>
    bool assert_eq(const T1& first, const T2& second) {
        return assert_test<T1, T2, AssertWhat::Eq>(first, second);
    }

    template <typename T1, typename T2, typename = EnableIfT<ConvertibleV<T1, T2>>>
    bool assert_non_eq(const T1& first, const T2& second) {
        return assert_test<T1, T2, AssertWhat::NonEq>(first, second);
    }

    template <typename T1, typename T2, typename = EnableIfT<ConvertibleV<T1, T2>>>
    bool assert_ptr_eq(const T1& first, const T2& second) {
        return assert_test<T1, T2, AssertWhat::PtrEq>(first, second);
    }

    template <typename T1, typename T2, typename = EnableIfT<ConvertibleV<T1, T2>>>
    bool assert_ptr_non_eq(const T1& first, const T2& second) {
        return assert_test<T1, T2, AssertWhat::PtrNonEq>(first, second);
    }
} // namespace ARLib

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

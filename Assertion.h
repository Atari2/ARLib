#pragma once
#include "Compat.h"
#include "Macros.h"
#include "SourceLocation.h"
#include "TypeTraits.h"

void abort_arlib();
void assertion_failed__();
void _assert_printf(const ARLib::SourceLocation& loc, const char*, ...);
void _assert_puts(const ARLib::SourceLocation& loc, const char*);
#if DEBUG_NEW_DELETE
    #undef PRINT_BACKTRACE
    #define PRINT_BACKTRACE()
#endif
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
}    // namespace ARLib
#ifdef ARLIB_DEBUG
    #define HARD_ASSERT(val, msg)                                                                                      \
        if (!(val)) {                                                                                                  \
            if (is_constant_evaluated()) {                                                                             \
                CONSTEVAL_STATIC_ASSERT(val, msg);                                                                     \
            } else {                                                                                                   \
                _assert_puts(ARLib::SourceLocation::current(), "ASSERTION \"" STRINGIFY(val) "\" FAILED: " msg);       \
                assertion_failed__();                                                                                  \
            }                                                                                                          \
            unreachable                                                                                                \
        }
    #define HARD_ASSERT_FMT(val, fmt, ...)                                                                             \
        if (!(val)) {                                                                                                  \
            if (is_constant_evaluated()) {                                                                             \
                CONSTEVAL_STATIC_ASSERT(val, fmt);                                                                     \
            } else {                                                                                                   \
                _assert_printf(                                                                                        \
                ARLib::SourceLocation::current(), "ASSERTION \"" STRINGIFY(val) "\" FAILED: " fmt "\n", __VA_ARGS__    \
                );                     \
                assertion_failed__();                                                                                  \
            }                                                                                                          \
            unreachable                                                                                                \
        }

    #define SOFT_ASSERT(val, msg)                                                                                      \
        if (!(val)) {                                                                                                  \
            if (is_constant_evaluated()) {                                                                             \
                CONSTEVAL_STATIC_ASSERT(val, msg);                                                                     \
            } else {                                                                                                   \
                _assert_puts(ARLib::SourceLocation::current(), "ASSERTION \"" STRINGIFY(val) "\" FAILED: " msg);                                         \
            }                                                                                                          \
        }
    #define SOFT_ASSERT_FMT(val, fmt, ...)                                                                             \
        if (!(val)) {                                                                                                  \
            if (is_constant_evaluated()) {                                                                             \
                CONSTEVAL_STATIC_ASSERT(val, fmt);                                                                     \
            } else {                                                                                                   \
                _assert_printf(                                                                                        \
                ARLib::SourceLocation::current(), "ASSERTION \"" STRINGIFY(val) "\" FAILED: " fmt "\n", __VA_ARGS__    \
                );                     \
            }                                                                                                          \
        }
#else
    #define HARD_ASSERT(val, msg)
    #define HARD_ASSERT_FMT(val, fmt, ...)
    #define SOFT_ASSERT(val, msg)
    #define SOFT_ASSERT_FMT(val, fmt, ...)
#endif

#define ASSERT_NOT_REACHED(msg)                                                                                        \
    {                                                                                                                  \
        if (is_constant_evaluated()) {                                                                                 \
            CONSTEVAL_STATIC_ASSERT(false, msg);                                                                       \
        } else {                                                                                                       \
            _assert_puts(ARLib::SourceLocation::current(), msg);                                                                                         \
            assertion_failed__();                                                                                      \
        }                                                                                                              \
        unreachable                                                                                                    \
    }
#define ASSERT_NOT_REACHED_FMT(fmt, ...)                                                                               \
    {                                                                                                                  \
        if (is_constant_evaluated()) {                                                                                 \
            CONSTEVAL_STATIC_ASSERT(false, fmt);                                                                       \
        } else {                                                                                                       \
            _assert_printf(ARLib::SourceLocation::current(), fmt "\n", __VA_ARGS__);                                                                     \
            assertion_failed__();                                                                                      \
        }                                                                                                              \
        unreachable                                                                                                    \
    }

#define TODO_CLS(cls)                                                                                                  \
    cls() { todo__(); };                                                                                               \
    static void todo__() { HARD_ASSERT(false, CONCAT(STRINGIFY(cls), " is not implemented yet")) }
#define TODO(func) HARD_ASSERT(false, CONCAT(STRINGIFY(func), " not implemented yet"))

#pragma once

#include "Macros.h"
namespace ARLib {
enum class ComparatorType { Equal, LessOrEqual, GreatorOrEqual, Less, Greater, NotEqual };
template <typename A, ComparatorType T>
struct Comparator {
    constexpr bool compare(const A&, const A&) const {
        COMPTIME_ASSERT("Don't manually override the ComparatorType template parameter")
        return false;
    }
};
template <typename A>
struct Comparator<A, ComparatorType::Equal> {
    constexpr bool compare(const A& a, const A& b) const { return a == b; }
};
template <typename A>
struct Comparator<A, ComparatorType::LessOrEqual> {
    constexpr bool compare(const A& a, const A& b) const { return a <= b; }
};
template <typename A>
struct Comparator<A, ComparatorType::GreatorOrEqual> {
    constexpr bool compare(const A& a, const A& b) const { return a >= b; }
};
template <typename A>
struct Comparator<A, ComparatorType::Less> {
    constexpr bool compare(const A& a, const A& b) const { return a < b; }
};
template <typename A>
struct Comparator<A, ComparatorType::Greater> {
    constexpr bool compare(const A& a, const A& b) const { return a > b; }
};
template <typename A>
struct Comparator<A, ComparatorType::NotEqual> {
    constexpr bool compare(const A& a, const A& b) const { return a != b; }
};
}    // namespace ARLib

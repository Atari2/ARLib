#pragma once

#include "Macros.h"

namespace ARLib {
    enum class ComparatorType { Equal, LessOrEqual, GreatorOrEqual, Less, Greater, NotEqual };

    template <typename A, ComparatorType T>
    struct Comparator {
        bool compare(const A&, const A&) const {
            COMPTIME_ASSERT("Don't manually override the ComparatorType template parameter")
        }
    };

    template <typename A>
    struct Comparator<A, ComparatorType::Equal> {
        bool compare(const A& a, const A& b) const { return a == b; }
    };

    template <typename A>
    struct Comparator<A, ComparatorType::LessOrEqual> {
        bool compare(const A& a, const A& b) const { return a <= b; }
    };
    template <typename A>
    struct Comparator<A, ComparatorType::GreatorOrEqual> {
        bool compare(const A& a, const A& b) const { return a >= b; }
    };
    template <typename A>
    struct Comparator<A, ComparatorType::Less> {
        bool compare(const A& a, const A& b) const { return a < b; }
    };
    template <typename A>
    struct Comparator<A, ComparatorType::Greater> {
        bool compare(const A& a, const A& b) const { return a > b; }
    };
    template <typename A>
    struct Comparator<A, ComparatorType::NotEqual> {
        bool compare(const A& a, const A& b) const { return a != b; }
    };

} // namespace ARLib
#pragma once
#include "Compat.h"
#include "Concepts.h"

// this is here so I can declare the proper constructor in the cpp file
// and avoid "#include <compare>" in here
// it's only needed because otherwise I can't use <=> in my DefaultOrdering<T> class in SortedVector<T>
namespace std {
    struct strong_ordering;
    struct partial_ordering;
}

namespace ARLib {
    enum class OrderingType { Less, Equal, Greater, NoOrder };

    class Ordering {
        OrderingType m_type;

        public:
        explicit Ordering(OrderingType type) noexcept : m_type(type) {}
        Ordering() = delete;
        Ordering(const Ordering&) = default;
        Ordering(Ordering&&) = default;
        Ordering& operator=(const Ordering&) = default;
        Ordering& operator=(Ordering&&) = default;
        forceinline OrderingType type() const { return m_type; }

        friend forceinline bool operator==(const Ordering& v, const Ordering& w) noexcept {
            return v.type() == w.type();
        }
        Ordering(const std::strong_ordering&);
        Ordering(const std::partial_ordering&);
    };

    static const inline Ordering less{OrderingType::Less};
    static const inline Ordering equal{OrderingType::Equal};
    static const inline Ordering greater{OrderingType::Greater};

    template <typename T>
    requires EqualityComparable<T>&& LessComparable<T>&& MoreComparable<T> constexpr Ordering
    CompareThreeWay(const T& a, const T& b) {
        if (a == b)
            return equal;
        else if (a < b)
            return less;
        else
            return greater;
    }

} // namespace ARLib

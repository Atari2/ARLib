#pragma once
#include "Compat.h"
#include "Concepts.h"
#include "EnumHelpers.h"
// this is here so I can declare the proper constructor in the cpp file
// and avoid "#include <compare>" in here
// it's only needed because otherwise I can't use <=> in my DefaultOrdering<T> class in SortedVector<T>
namespace std {
struct strong_ordering;
struct partial_ordering;
}    // namespace std
namespace ARLib {
enum class OrderingType { Less = -1, Equal = 0, Greater = 1, NoOrder = 2 };
class Ordering {
    OrderingType m_type;
    using ZeroValue = decltype(nullptr);
    public:
    explicit Ordering(OrderingType type) noexcept : m_type(type) {}
    Ordering()                           = delete;
    Ordering(const Ordering&)            = default;
    Ordering(Ordering&&)                 = default;
    Ordering& operator=(const Ordering&) = default;
    Ordering& operator=(Ordering&&)      = default;
    forceinline OrderingType type() const { return m_type; }
    friend forceinline bool operator==(const Ordering v, const Ordering w) noexcept { return v.type() == w.type(); }
    friend forceinline bool operator==(const Ordering v, ZeroValue) noexcept { return from_enum(v.type()) == 0; }
    friend forceinline bool operator==(ZeroValue, const Ordering v) noexcept { return from_enum(v.type()) == 0; }
    friend forceinline bool operator<(const Ordering v, const Ordering w) noexcept { return v.type() < w.type(); }
    friend forceinline bool operator<(const Ordering v, ZeroValue) noexcept { return from_enum(v.type()) < 0; }
    friend forceinline bool operator<(ZeroValue, const Ordering v) noexcept { return from_enum(v.type()) < 0; }
    friend forceinline bool operator<=(const Ordering v, const Ordering w) noexcept { return v.type() <= w.type(); }
    friend forceinline bool operator<=(const Ordering v, ZeroValue) noexcept { return from_enum(v.type()) <= 0; }
    friend forceinline bool operator<=(ZeroValue, const Ordering v) noexcept { return from_enum(v.type()) <= 0; }
    friend forceinline bool operator>(const Ordering v, const Ordering w) noexcept { return v.type() > w.type(); }
    friend forceinline bool operator>(const Ordering v, ZeroValue) noexcept { return from_enum(v.type()) > 0; }
    friend forceinline bool operator>(ZeroValue, const Ordering v) noexcept { return from_enum(v.type()) > 0; }
    friend forceinline bool operator>=(const Ordering v, const Ordering w) noexcept { return v.type() >= w.type(); }
    friend forceinline bool operator>=(const Ordering v, ZeroValue) noexcept { return from_enum(v.type()) >= 0; }
    friend forceinline bool operator>=(ZeroValue, const Ordering v) noexcept { return from_enum(v.type()) >= 0; }
    friend forceinline Ordering operator<=>(const Ordering v, const Ordering w) noexcept {
        const auto a = from_enum(v.type());
        const auto b = from_enum(w.type());
        if (a == b)
            return Ordering{ OrderingType::Equal };
        else if (a < b)
            return Ordering{ OrderingType::Less };
        else
            return Ordering{ OrderingType::Greater };
    }
    friend forceinline Ordering operator<=>(const Ordering v, ZeroValue) noexcept { return v; }
    friend forceinline Ordering operator<=>(ZeroValue, const Ordering v) noexcept {
        return Ordering{ to_enum<OrderingType>(0 - from_enum(v.type())) };
    }
    Ordering(const std::strong_ordering&);
    Ordering(const std::partial_ordering&);
};
static inline const Ordering less{ OrderingType::Less };
static inline const Ordering equal{ OrderingType::Equal };
static inline const Ordering greater{ OrderingType::Greater };
template <typename T>
requires EqualityComparable<T> && LessComparable<T> && MoreComparable<T>
constexpr Ordering CompareThreeWay(const T& a, const T& b) {
    if (a == b)
        return equal;
    else if (a < b)
        return less;
    else
        return greater;
}
}    // namespace ARLib

#pragma once
#include "Compat.hpp"
#include "Concepts.hpp"
#include "Conversion.hpp"
#include <compare>
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
    arlib_forceinline OrderingType type() const { return m_type; }
    // equal
    friend inline bool operator==(std::strong_ordering v, const Ordering w) noexcept { return Ordering{ v } == w; }
    friend inline bool operator==(const Ordering v, std::strong_ordering w) noexcept { return v == Ordering{ w }; }
    friend inline bool operator==(std::partial_ordering v, const Ordering w) noexcept { return Ordering{ v } == w; }
    friend inline bool operator==(const Ordering v, std::partial_ordering w) noexcept { return v == Ordering{ w }; }
    // less
    friend inline bool operator<(std::strong_ordering v, const Ordering w) noexcept { return Ordering{ v } < w; }
    friend inline bool operator<(const Ordering v, std::strong_ordering w) noexcept { return v < Ordering{ w }; }
    friend inline bool operator<(std::partial_ordering v, const Ordering w) noexcept { return Ordering{ v } < w; }
    friend inline bool operator<(const Ordering v, std::partial_ordering w) noexcept { return v < Ordering{ w }; }
    // less-equal
    friend inline bool operator<=(std::strong_ordering v, const Ordering w) noexcept { return Ordering{ v } <= w; }
    friend inline bool operator<=(const Ordering v, std::strong_ordering w) noexcept { return v <= Ordering{ w }; }
    friend inline bool operator<=(std::partial_ordering v, const Ordering w) noexcept { return Ordering{ v } <= w; }
    friend inline bool operator<=(const Ordering v, std::partial_ordering w) noexcept { return v <= Ordering{ w }; }
    // greater
    friend inline bool operator>(std::strong_ordering v, const Ordering w) noexcept { return Ordering{ v } > w; }
    friend inline bool operator>(const Ordering v, std::strong_ordering w) noexcept { return v > Ordering{ w }; }
    friend inline bool operator>(std::partial_ordering v, const Ordering w) noexcept { return Ordering{ v } > w; }
    friend inline bool operator>(const Ordering v, std::partial_ordering w) noexcept { return v > Ordering{ w }; }
    // greater-equal
    friend inline bool operator>=(std::strong_ordering v, const Ordering w) noexcept { return Ordering{ v } >= w; }
    friend inline bool operator>=(const Ordering v, std::strong_ordering w) noexcept { return v >= Ordering{ w }; }
    friend inline bool operator>=(std::partial_ordering v, const Ordering w) noexcept { return Ordering{ v } >= w; }
    friend inline bool operator>=(const Ordering v, std::partial_ordering w) noexcept { return v >= Ordering{ w }; }
    // spaceship
    friend inline Ordering operator<=>(std::strong_ordering v, const Ordering w) noexcept {
        return Ordering{ v } <=> w;
    }
    friend inline Ordering operator<=>(const Ordering v, std::strong_ordering w) noexcept {
        return v <=> Ordering{ w };
    }
    friend inline Ordering operator<=>(std::partial_ordering v, const Ordering w) noexcept {
        return Ordering{ v } <=> w;
    }
    friend inline Ordering operator<=>(const Ordering v, std::partial_ordering w) noexcept {
        return v <=> Ordering{ w };
    }
    friend arlib_forceinline bool operator==(const Ordering v, const Ordering w) noexcept {
        return v.type() == w.type();
    }
    friend arlib_forceinline bool operator==(const Ordering v, ZeroValue) noexcept { return from_enum(v.type()) == 0; }
    friend arlib_forceinline bool operator==(ZeroValue, const Ordering v) noexcept { return from_enum(v.type()) == 0; }
    friend arlib_forceinline bool operator<(const Ordering v, const Ordering w) noexcept { return v.type() < w.type(); }
    friend arlib_forceinline bool operator<(const Ordering v, ZeroValue) noexcept { return from_enum(v.type()) < 0; }
    friend arlib_forceinline bool operator<(ZeroValue, const Ordering v) noexcept { return from_enum(v.type()) < 0; }
    friend arlib_forceinline bool operator<=(const Ordering v, const Ordering w) noexcept {
        return v.type() <= w.type();
    }
    friend arlib_forceinline bool operator<=(const Ordering v, ZeroValue) noexcept { return from_enum(v.type()) <= 0; }
    friend arlib_forceinline bool operator<=(ZeroValue, const Ordering v) noexcept { return from_enum(v.type()) <= 0; }
    friend arlib_forceinline bool operator>(const Ordering v, const Ordering w) noexcept { return v.type() > w.type(); }
    friend arlib_forceinline bool operator>(const Ordering v, ZeroValue) noexcept { return from_enum(v.type()) > 0; }
    friend arlib_forceinline bool operator>(ZeroValue, const Ordering v) noexcept { return from_enum(v.type()) > 0; }
    friend arlib_forceinline bool operator>=(const Ordering v, const Ordering w) noexcept {
        return v.type() >= w.type();
    }
    friend arlib_forceinline bool operator>=(const Ordering v, ZeroValue) noexcept { return from_enum(v.type()) >= 0; }
    friend arlib_forceinline bool operator>=(ZeroValue, const Ordering v) noexcept { return from_enum(v.type()) >= 0; }
    friend arlib_forceinline Ordering operator<=>(const Ordering v, const Ordering w) noexcept {
        const auto a = from_enum(v.type());
        const auto b = from_enum(w.type());
        if (a == b)
            return Ordering{ OrderingType::Equal };
        else if (a < b)
            return Ordering{ OrderingType::Less };
        else
            return Ordering{ OrderingType::Greater };
    }
    friend arlib_forceinline Ordering operator<=>(const Ordering v, ZeroValue) noexcept { return v; }
    friend arlib_forceinline Ordering operator<=>(ZeroValue, const Ordering v) noexcept {
        return Ordering{ to_enum<OrderingType>(0 - from_enum(v.type())) };
    }
    Ordering(const std::strong_ordering&);
    Ordering(const std::partial_ordering&);
};
static inline const Ordering less{ OrderingType::Less };
static inline const Ordering equal{ OrderingType::Equal };
static inline const Ordering greater{ OrderingType::Greater };
static inline const Ordering unordered{ OrderingType::NoOrder };
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

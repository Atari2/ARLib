#pragma once

#include <Concepts.hpp>
#include <Types.hpp>

namespace ARLib {
struct DefaultInitTag {};
struct ValueInitTag {};
template <typename T, size_t Idx, bool CanBeEmpty = IsEmptyV<T> && !IsFinalV<T>>
class CompressedPairElem {
    T elem;

    public:
    constexpr explicit CompressedPairElem(DefaultInitTag) {}
    constexpr explicit CompressedPairElem(ValueInitTag) : elem{} {}
    template <typename U>
    requires(!SameAs<CompressedPairElem, DecayT<U>>)
    constexpr explicit CompressedPairElem(U&& u) : elem{ Forward<U>(u) } {}
    T& get() noexcept { return elem; }
    const T& get() const noexcept { return elem; }
};
template <typename T, size_t Idx>
struct CompressedPairElem<T, Idx, true> : private T {
    constexpr explicit CompressedPairElem() = default;
    constexpr explicit CompressedPairElem(DefaultInitTag) {}
    constexpr explicit CompressedPairElem(ValueInitTag) : T{} {}
    template <typename U>
    requires(!SameAs<CompressedPairElem, DecayT<U>>)
    constexpr explicit CompressedPairElem(U&& u) : T{ Forward<U>(u) } {}
    T& get() noexcept { return *this; }
    const T& get() const noexcept { return *this; }
};
template <typename T1, typename T2>
class CompressedPair : private CompressedPairElem<T1, 0>, private CompressedPairElem<T2, 1> {
    using B1 = CompressedPairElem<T1, 0>;
    using B2 = CompressedPairElem<T2, 0>;
    public:
    constexpr explicit CompressedPair() : B1{ ValueInitTag{} }, B2{ ValueInitTag{} } {};
    template <typename U1, typename U2>
    constexpr explicit CompressedPair(U1&& u1, U2&& u2) : B1{ Forward<U1>(u1) }, B2{ Forward<U2>(u2) } {}
    T1& first() noexcept { return static_cast<B1&>(*this).get(); }
    T1& first() const noexcept { return static_cast<const B1&>(*this).get(); }
    T2& second() noexcept { return static_cast<B2&>(*this).get(); }
    T2& second() const noexcept { return static_cast<const B2&>(*this).get(); }
};
template <typename T1, typename T2>
CompressedPair(T1, T2) -> CompressedPair<T1, T2>;

}    // namespace ARLib
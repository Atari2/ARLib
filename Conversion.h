#pragma once
#include "Assertion.h"
#include "Concepts.h"
#include "Types.h"
namespace ARLib {
template <typename A, typename B>
A cast(B value) {
    if constexpr (ConvertibleTo<A, B>) {
        return static_cast<A>(value);
    } else if constexpr (IsPointerV<A> && IsPointerV<B>) {
        return reinterpret_cast<A>(value);
    } else {
        COMPTIME_ASSERT("Value is not castable in a safe manner, please do it manually");
    }
}
template <typename A>
A* to_ptr(uintptr_t address) {
    return reinterpret_cast<A*>(address);
}
template <typename A>
uintptr_t from_ptr(A* ptr) {
    return reinterpret_cast<uintptr_t>(ptr);
}
template <typename E>
requires IsEnumV<E>
constexpr E to_enum(UnderlyingTypeT<E> value) {
    return static_cast<E>(value);
}
template <typename E>
requires IsEnumV<E>
constexpr UnderlyingTypeT<E> from_enum(E value) {
    return static_cast<UnderlyingTypeT<E>>(value);
}
}    // namespace ARLib

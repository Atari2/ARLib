#pragma once
#include "Iterator.h"
#include "PrintInfo.h"
#include "Types.h"
#include "Utility.h"
#include "TupleDestructuring.h"
namespace ARLib {
template <typename T, typename U>
struct Pair {
    T _m_first;
    U _m_second;

    template <typename Ty, typename Uy>
    constexpr static inline bool PairConstructible = Constructible<T, Ty> && Constructible<U, Uy>;
    template <typename Ty, typename Uy>
    constexpr static inline bool PairConvertibleTo = ConvertibleTo<Ty, T> && ConvertibleTo<Uy, U>;

    constexpr Pair() = default;
    template <typename Ty, typename Uy>
    requires(PairConvertibleTo<Ty, Uy> && !PairConstructible<Ty, Uy>)
    constexpr Pair(Ty first, Uy second) : _m_first(static_cast<T>(first)), _m_second(static_cast<U>(second)) {}
    template <typename Ty, typename Uy>
    requires(PairConstructible<Ty, Uy> && !PairConvertibleTo<Ty, Uy>)
    constexpr Pair(Ty first, Uy second) : _m_first(first), _m_second(second) {}
    constexpr Pair(T&& first, U&& second) : _m_first(Forward<T>(first)), _m_second(Forward<U>(second)) {}
    constexpr Pair(const T& first, const U& second) : _m_first(first), _m_second(second) {}
    constexpr Pair(const Pair&)                = default;
    constexpr Pair(Pair&&) noexcept            = default;
    constexpr Pair& operator=(Pair&&) noexcept = default;
    constexpr Pair& operator=(const Pair&)     = default;
    constexpr bool operator==(const Pair& other) const {
        return _m_first == other._m_first && _m_second == other._m_second;
    }
    constexpr bool operator!=(const Pair& other) const {
        return _m_first != other._m_first || _m_second != other._m_second;
    }
    constexpr T& first() { return _m_first; }
    constexpr U& second() { return _m_second; }
    constexpr const T& first() const { return _m_first; }
    constexpr const U& second() const { return _m_second; }
    template <size_t Index>
    auto& get() & {
        if constexpr (IsTupleV<T> && IsTupleV<U>) {
            constexpr size_t tsize = std::tuple_size<T>::value;
            constexpr size_t usize = std::tuple_size<U>::value;
            static_assert(Index < (tsize + usize), "Invalid index passed to pair<get>");
            if constexpr (Index < tsize) {
                return _m_first.template get<Index>();
            } else {
                return _m_second.template get<Index - tsize>();
            }
        } else if constexpr (IsTupleV<T>) {
            constexpr size_t tsize = std::tuple_size<T>::value;
            constexpr size_t usize = 1;
            static_assert(Index < (tsize + usize), "Invalid index passed to pair<get>");
            if constexpr (Index < tsize) {
                return _m_first.template get<Index>();
            } else {
                return _m_second;
            }
        } else if constexpr (IsTupleV<U>) {
            constexpr size_t tsize = 1;
            constexpr size_t usize = std::tuple_size<U>::value;
            static_assert(Index < (tsize + usize), "Invalid index passed to pair<get>");
            if constexpr (Index < tsize) {
                return _m_first;
            } else {
                return _m_second.template get<Index - tsize>();
            }
        } else {
            static_assert(Index == 0 || Index == 1);
            if constexpr (Index == 0)
                return _m_first;
            else if constexpr (Index == 1)
                return _m_second;
        }
    }
    template <size_t Index>
    const auto& get() const& {
        if constexpr (IsTupleV<T> && IsTupleV<U>) {
            constexpr size_t tsize = std::tuple_size<T>::value;
            constexpr size_t usize = std::tuple_size<U>::value;
            static_assert(Index < (tsize + usize), "Invalid index passed to pair<get>");
            if constexpr (Index < tsize) {
                return _m_first.template get<Index>();
            } else {
                return _m_second.template get<Index - tsize>();
            }
        } else if constexpr (IsTupleV<T>) {
            constexpr size_t tsize = std::tuple_size<T>::value;
            constexpr size_t usize = 1;
            static_assert(Index < (tsize + usize), "Invalid index passed to pair<get>");
            if constexpr (Index < tsize) {
                return _m_first.template get<Index>();
            } else {
                return _m_second;
            }
        } else if constexpr (IsTupleV<U>) {
            constexpr size_t tsize = 1;
            constexpr size_t usize = std::tuple_size<U>::value;
            static_assert(Index < (tsize + usize), "Invalid index passed to pair<get>");
            if constexpr (Index < tsize) {
                return _m_first;
            } else {
                return _m_second.template get<Index - tsize>();
            }
        } else {
            static_assert(Index == 0 || Index == 1);
            if constexpr (Index == 0)
                return _m_first;
            else if constexpr (Index == 1)
                return _m_second;
        }
    }
    template <size_t Index>
    auto&& get() && {
        if constexpr (IsTupleV<T> && IsTupleV<U>) {
            constexpr size_t tsize = std::tuple_size<T>::value;
            constexpr size_t usize = std::tuple_size<U>::value;
            static_assert(Index < (tsize + usize), "Invalid index passed to pair<get>");
            if constexpr (Index < tsize) {
                return move(_m_first.template get<Index>());
            } else {
                return move(_m_second.template get<Index - tsize>());
            }
        } else if constexpr (IsTupleV<T>) {
            constexpr size_t tsize = std::tuple_size<T>::value;
            constexpr size_t usize = 1;
            static_assert(Index < (tsize + usize), "Invalid index passed to pair<get>");
            if constexpr (Index < tsize) {
                return move(_m_first.template get<Index>());
            } else {
                return move(_m_second);
            }
        } else if constexpr (IsTupleV<U>) {
            constexpr size_t tsize = 1;
            constexpr size_t usize = std::tuple_size<U>::value;
            static_assert(Index < (tsize + usize), "Invalid index passed to pair<get>");
            if constexpr (Index < tsize) {
                return move(_m_first);
            } else {
                return move(_m_second.template get<Index - tsize>());
            }
        } else {
            static_assert(Index == 0 || Index == 1);
            if constexpr (Index == 0)
                return move(_m_first);
            else if constexpr (Index == 1)
                return move(_m_second);
        }
    }
    ~Pair() = default;
};
template <typename T, typename U>
struct Pair<T&, U&> {
    T& _m_first;
    U& _m_second;
    Pair(T& first, U& second) : _m_first(first), _m_second(second) {}
    Pair(const Pair& other) : _m_first(other._m_first), _m_second(other._m_second){};
    Pair(Pair&& other) noexcept : _m_first(other._m_first), _m_second(other._m_second){};
    T& first() { return _m_first; }
    U& second() { return _m_second; }
    const T& first() const { return _m_first; }
    const U& second() const { return _m_second; }
    template <size_t Index>
    auto& get() & {
        if constexpr (IsTupleV<T> && IsTupleV<U>) {
            constexpr size_t tsize = std::tuple_size<T>::value;
            constexpr size_t usize = std::tuple_size<U>::value;
            static_assert(Index < (tsize + usize), "Invalid index passed to pair<get>");
            if constexpr (Index < tsize) {
                return _m_first.template get<Index>();
            } else {
                return _m_second.template get<Index - tsize>();
            }
        } else if constexpr (IsTupleV<T>) {
            constexpr size_t tsize = std::tuple_size<T>::value;
            constexpr size_t usize = 1;
            static_assert(Index < (tsize + usize), "Invalid index passed to pair<get>");
            if constexpr (Index < tsize) {
                return _m_first.template get<Index>();
            } else {
                return _m_second;
            }
        } else if constexpr (IsTupleV<U>) {
            constexpr size_t tsize = 1;
            constexpr size_t usize = std::tuple_size<U>::value;
            static_assert(Index < (tsize + usize), "Invalid index passed to pair<get>");
            if constexpr (Index < tsize) {
                return _m_first;
            } else {
                return _m_second.template get<Index - tsize>();
            }
        } else {
            static_assert(Index == 0 || Index == 1);
            if constexpr (Index == 0)
                return _m_first;
            else if constexpr (Index == 1)
                return _m_second;
        }
    }
    template <size_t Index>
    const auto& get() const& {
        if constexpr (IsTupleV<T> && IsTupleV<U>) {
            constexpr size_t tsize = std::tuple_size<T>::value;
            constexpr size_t usize = std::tuple_size<U>::value;
            static_assert(Index < (tsize + usize), "Invalid index passed to pair<get>");
            if constexpr (Index < tsize) {
                return _m_first.template get<Index>();
            } else {
                return _m_second.template get<Index - tsize>();
            }
        } else if constexpr (IsTupleV<T>) {
            constexpr size_t tsize = std::tuple_size<T>::value;
            constexpr size_t usize = 1;
            static_assert(Index < (tsize + usize), "Invalid index passed to pair<get>");
            if constexpr (Index < tsize) {
                return _m_first.template get<Index>();
            } else {
                return _m_second;
            }
        } else if constexpr (IsTupleV<U>) {
            constexpr size_t tsize = 1;
            constexpr size_t usize = std::tuple_size<U>::value;
            static_assert(Index < (tsize + usize), "Invalid index passed to pair<get>");
            if constexpr (Index < tsize) {
                return _m_first;
            } else {
                return _m_second.template get<Index - tsize>();
            }
        } else {
            static_assert(Index == 0 || Index == 1);
            if constexpr (Index == 0)
                return _m_first;
            else if constexpr (Index == 1)
                return _m_second;
        }
    }
    template <size_t Index>
    auto&& get() && {
        if constexpr (IsTupleV<T> && IsTupleV<U>) {
            constexpr size_t tsize = std::tuple_size<T>::value;
            constexpr size_t usize = std::tuple_size<U>::value;
            static_assert(Index < (tsize + usize), "Invalid index passed to pair<get>");
            if constexpr (Index < tsize) {
                return move(_m_first.template get<Index>());
            } else {
                return move(_m_second.template get<Index - tsize>());
            }
        } else if constexpr (IsTupleV<T>) {
            constexpr size_t tsize = std::tuple_size<T>::value;
            constexpr size_t usize = 1;
            static_assert(Index < (tsize + usize), "Invalid index passed to pair<get>");
            if constexpr (Index < tsize) {
                return move(_m_first.template get<Index>());
            } else {
                return move(_m_second);
            }
        } else if constexpr (IsTupleV<U>) {
            constexpr size_t tsize = 1;
            constexpr size_t usize = std::tuple_size<U>::value;
            static_assert(Index < (tsize + usize), "Invalid index passed to pair<get>");
            if constexpr (Index < tsize) {
                return move(_m_first);
            } else {
                return move(_m_second.template get<Index - tsize>());
            }
        } else {
            static_assert(Index == 0 || Index == 1);
            if constexpr (Index == 0)
                return move(_m_first);
            else if constexpr (Index == 1)
                return move(_m_second);
        }
    }
    ~Pair() = default;
};
template <typename A, typename B>
struct PrintInfo<Pair<A, B>> {
    const Pair<A, B>& m_pair;
    explicit PrintInfo(const Pair<A, B>& pair) : m_pair(pair) {}
    String repr() const {
        return "{ "_s + print_conditional<A>(m_pair.first()) + ", "_s + print_conditional<B>(m_pair.second()) + " }"_s;
    }
};
}    // namespace ARLib

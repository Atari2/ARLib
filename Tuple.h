#pragma once
#include "Concepts.h"
#include "Invoke.h"
#include "PrintInfo.h"
#include "Utility.h"

// we love UB
// forward declaring things in std:: is UB
// but this avoids having to #include <utility>
// so I'll do it
namespace std {
template <class T>
struct tuple_size;
template <size_t I, class T>
struct tuple_element;
}

namespace ARLib {
template <typename T, typename... Args>
class Tuple : Tuple<Args...> {
    T m_member;
    template <size_t N>
    bool equality_impl(const Tuple& other) const {
        if constexpr (N == 0)
            return m_member == other.m_member;
        else {
            if (this->template get<N>() == other.template get<N>()) {
                return equality_impl<N - 1>(other);
            } else {
                return false;
            }
        }
    }
    template <size_t N>
    bool inequality_impl(const Tuple& other) const {
        if constexpr (N == 0)
            return m_member != other.m_member;
        else {
            if (this->template get<N>() == other.template get<N>()) {
                return inequality_impl<N - 1>(other);
            } else {
                return true;
            }
        }
    }

    public:
    constexpr static inline size_t size = 1 + sizeof...(Args);
    Tuple()                             = default;
    Tuple(const Tuple&)                 = default;
    Tuple(Tuple&&) noexcept             = default;
    explicit Tuple(T arg, Args... args) : Tuple<Args...>(args...), m_member(arg) {}
    Tuple& operator=(const Tuple&)     = default;
    Tuple& operator=(Tuple&&) noexcept = default;
    bool operator==(const Tuple& other) const { return equality_impl<sizeof...(Args) - 1>(other); }
    bool operator!=(const Tuple& other) const { return inequality_impl<sizeof...(Args) - 1>(other); }
    Tuple& operator=(T&& value) {
        static_assert(
        !IsAnyOfCvRefV<T, Args...>,
        "Don't use the assignment operator with a type that appears more than once in the tuple"
        );
        m_member = move(value);
        return *this;
    }
    template <typename U>
    requires IsAnyOfV<U, Args...>
    Tuple& operator=(U&& value) {
        static_cast<Tuple<Args...>*>(this)->template set<U>(Forward<U>(value));
        return *this;
    }
    template <typename Tp>
    constexpr const auto& get() const {
        static_assert(IsAnyOfCvRefV<Tp, T, Args...>);
        if constexpr (SameAsCvRef<Tp, RemoveReferenceT<T>>) {
            static_assert(
            !IsAnyOfCvRefV<Tp, RemoveReferenceT<Args>...>,
            "Don't use the typed get function with a type that appears more than once in the tuple"
            );
            return m_member;
        } else {
            return static_cast<const Tuple<Args...>*>(this)->template get<Tp>();
        }
    }
    template <typename Tp>
    constexpr auto& get() {
        static_assert(IsAnyOfCvRefV<Tp, T, Args...>);
        if constexpr (SameAsCvRef<Tp, T>) {
            static_assert(
            !IsAnyOfCvRefV<Tp, Args...>,
            "Don't use the typed get function with a type that appears more than once in the tuple"
            );
            return m_member;
        } else {
            return static_cast<Tuple<Args...>*>(this)->template get<Tp>();
        }
    }
    template <size_t N>
    constexpr const auto& get() const {
        if constexpr (N == 0) {
            return m_member;
        } else {
            static_assert(N < (sizeof...(Args) + 1) && N > 0);
            return static_cast<const Tuple<Args...>*>(this)->template get<N - 1>();
        }
    }
    template <size_t N>
    constexpr auto& get() {
        if constexpr (N == 0) {
            return m_member;
        } else {
            static_assert(N < (sizeof...(Args) + 1) && N > 0);
            return static_cast<Tuple<Args...>*>(this)->template get<N - 1>();
        }
    }
    template <typename Tp>
    constexpr void set(Tp&& p) {
        static_assert(IsAnyOfCvRefV<Tp, T, Args...>);
        if constexpr (SameAsCvRef<Tp, T>) {
            m_member = move(p);
        } else {
            static_cast<Tuple<Args...>*>(this)->template set<Tp>(Forward<Tp>(p));
        }
    }
    template <size_t N, typename F>
    constexpr void set(F&& f) {
        if constexpr (N == 0) {
            m_member = move(f);
        } else {
            static_assert(N < (sizeof...(Args) + 1) && N > 0);
            static_cast<Tuple<Args...>*>(this)->template set<N - 1, F>(Forward<F>(f));
        }
    }
};
template <typename T>
class Tuple<T> {
    T m_member;

    public:
    constexpr static inline size_t size = 1;
    Tuple()                             = default;
    explicit Tuple(T arg) : m_member(arg) {}
    bool operator==(const Tuple& other) const { return m_member == other.m_member; }
    Tuple& operator=(T&& value) {
        m_member = move(value);
        return *this;
    }
    template <typename Tp>
    auto& get() {
        static_assert(SameAsCvRef<Tp, T>);
        return m_member;
    }
    template <typename Tp>
    const auto& get() const {
        static_assert(SameAsCvRef<Tp, T>);
        return m_member;
    }
    template <size_t N>
    constexpr T& get() {
        static_assert(N == 0);
        return m_member;
    }
    template <size_t N>
    constexpr const T& get() const {
        static_assert(N == 0);
        return m_member;
    }
    template <typename Tp>
    void set(Tp&& p) {
        static_assert(SameAsCvRef<Tp, T>);
        m_member = move(p);
    }
    template <size_t N, typename _>
    constexpr void set(T&& f) {
        static_assert(N == 0);
        m_member = move(f);
    }
};
// free functions to avoid template keyword when calling member functions in templated functions.
template <typename Tp, typename... Args>
requires IsAnyOfV<Tp, Args...>
auto& get(Tuple<Args...>& tuple) {
    return tuple.template get<Tp>();
}
template <typename Tp, typename... Args>
requires IsAnyOfV<Tp, Args...>
const auto& get(const Tuple<Args...>& tuple) {
    return tuple.template get<Tp>();
}
template <size_t Idx, typename... Args>
requires(Idx < sizeof...(Args))
auto& get(Tuple<Args...>& tuple) {
    return tuple.template get<Idx>();
}
template <size_t Idx, typename... Args>
requires(Idx < sizeof...(Args))
const auto& get(const Tuple<Args...>& tuple) {
    return tuple.template get<Idx>();
}
template <typename Tp, typename... Args>
requires IsAnyOfV<Tp, Args...>
void set(Tuple<Args...>& tuple, Tp value) {
    tuple.template set<Tp>(move(value));
}
template <size_t Idx, typename Tp, typename... Args>
requires(Idx < sizeof...(Args) && IsAnyOfV<Tp, Args...>)
void set(Tuple<Args...>& tuple, Tp value) {
    tuple.template set<Idx, Tp>(move(value));
}
template <typename... Args>
constexpr Tuple<Args&...> tie(Args&... args) noexcept {
    return Tuple<Args&...>{ args... };
}
template <class Tuple>
struct TupleSize {
    constexpr static inline size_t value = Tuple::size;
};

template <class Tuple>
constexpr inline size_t TupleSizeV = TupleSize<Tuple>::value;
namespace detail {
    template <class F, class Tuple, size_t... I>
    constexpr decltype(auto) apply_impl(F&& f, Tuple&& t, IndexSequence<I...>) {
        return invoke(Forward<F>(f), get<I>(Forward<Tuple>(t))...);
    }
}    // namespace detail
template <class F, class Tuple>
constexpr decltype(auto) apply(F&& f, Tuple&& t) {
    return detail::apply_impl(
    Forward<F>(f), Forward<Tuple>(t), MakeIndexSequence<TupleSizeV<RemoveReferenceT<Tuple>>>{}
    );
}
template <typename... Args>
requires(Printable<RemoveCvRefT<Args>>, ...)
struct PrintInfo<Tuple<Args...>> {
    const Tuple<Args...>& m_tuple;
    explicit PrintInfo(const Tuple<Args...>& tuple) : m_tuple(tuple) {}
    String repr() const {
        String str{ "{ " };
        (str.append(print_conditional(get<Args>(m_tuple)) + ", "_s), ...);
        str = str.substring(0, str.size() - 2);
        str.append(" }");
        return str;
    }
};
}    // namespace ARLib

// tuple_size and tuple_element specializations for ARLib::Tuple
template <typename... Types>
struct std::tuple_size<ARLib::Tuple<Types...>> : ARLib::IntegralConstant<ARLib::size_t, sizeof...(Types)> {};
template <typename... Types>
struct std::tuple_size<const ARLib::Tuple<Types...>> : ARLib::IntegralConstant<ARLib::size_t, sizeof...(Types)> {};

template <std::size_t I, class Head, class... Tail>
struct std::tuple_element<I, ARLib::Tuple<Head, Tail...>> : std::tuple_element<I - 1, ARLib::Tuple<Tail...>> {};
template <class Head, class... Tail>
struct std::tuple_element<0, ARLib::Tuple<Head, Tail...>> {
    using type = const Head;
};
template <std::size_t I, class Head, class... Tail>
struct std::tuple_element<I, const ARLib::Tuple<Head, Tail...>> : std::tuple_element<I - 1, ARLib::Tuple<Tail...>> {};
template <class Head, class... Tail>
struct std::tuple_element<0, const ARLib::Tuple<Head, Tail...>> {
    using type = const Head;
};

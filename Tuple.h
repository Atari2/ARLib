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
#ifdef _LIBCPP_VERSION
inline namespace __1 {
#endif
    template <class T>
    struct tuple_size;
    template <size_t I, class T>
    struct tuple_element;
#ifdef _LIBCPP_VERSION
}
#endif
}    // namespace std
namespace ARLib {
template <typename T, typename... Args>
class Tuple;
template <typename Tp, typename... P1, typename... P2>
auto get_tuple_base_array(TypeArray<Tp, P1...>, TypeArray<P2...>) {
    if constexpr (sizeof...(P1) == 0) {
        using A2 = TypeArray<P2..., Tuple<Tp>>;
        return A2{};
    } else {
        using A2 = TypeArray<P2..., Tuple<Tp, P1...>>;
        using A1 = TypeArray<P1...>;
        return get_tuple_base_array(A1{}, A2{});
    }
}
template <typename Tp, typename... Tps>
auto get_tuple_base_array_start() {
    if constexpr (sizeof...(Tps) == 0) {
        using A2 = TypeArray<Tuple<Tp>>;
        return A2{};
    } else {
        using A1 = TypeArray<Tps...>;
        using A2 = TypeArray<Tuple<Tp, Tps...>>;
        return get_tuple_base_array(A1{}, A2{});
    }
}
template <typename T, typename... Args>
class Tuple : public Tuple<Args...> {
    using BaseTuple      = Tuple<Args...>;
    using TupleBaseArray = decltype(get_tuple_base_array_start<T, Args...>());
    T m_member;
    template <size_t N>
    constexpr bool equality_impl(const Tuple& other) const {
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
    constexpr bool inequality_impl(const Tuple& other) const {
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
    constexpr Tuple()                   = default;
    constexpr Tuple(const Tuple&)       = default;
    constexpr Tuple(Tuple&&) noexcept   = default;
    constexpr explicit Tuple(const T& arg, const Args&... args) : Tuple<Args...>(args...), m_member(arg) {}
    template <typename U, typename... Yrgs>
    constexpr explicit Tuple(U&& arg, Yrgs&&... args) :
        Tuple<Args...>(Forward<Yrgs>(args)...), m_member(Forward<U>(arg)) {}
    constexpr Tuple& operator=(const Tuple&)     = default;
    constexpr Tuple& operator=(Tuple&&) noexcept = default;
    constexpr bool operator==(const Tuple& other) const { return equality_impl<sizeof...(Args) - 1>(other); }
    constexpr bool operator!=(const Tuple& other) const { return inequality_impl<sizeof...(Args) - 1>(other); }
    constexpr Tuple& operator=(T&& value) {
        static_assert(
        !IsAnyOfCvRefV<T, Args...>,
        "Don't use the assignment operator with a type that appears more than once in the tuple"
        );
        m_member = move(value);
        return *this;
    }
    template <typename U>
    requires IsAnyOfV<U, Args...>
    constexpr Tuple& operator=(U&& value) {
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
    using TupleBaseArray = decltype(get_tuple_base_array_start<T>());
    public:
    constexpr static inline size_t size = 1;
    constexpr Tuple()                   = default;
    constexpr explicit Tuple(const T& arg) : m_member(arg) {}
    template <typename U>
    constexpr explicit Tuple(U&& arg) : m_member(Forward<U>(arg)) {}
    constexpr bool operator==(const Tuple& other) const { return m_member == other.m_member; }
    constexpr Tuple& operator=(T&& value) {
        m_member = move(value);
        return *this;
    }
    template <typename Tp>
    constexpr auto& get() {
        static_assert(SameAsCvRef<Tp, T>);
        return m_member;
    }
    template <typename Tp>
    constexpr const auto& get() const {
        static_assert(SameAsCvRef<Tp, T>);
        return m_member;
    }
    template <size_t N>
    constexpr auto& get() {
        static_assert(N == 0);
        return m_member;
    }
    template <size_t N>
    constexpr const auto& get() const {
        static_assert(N == 0);
        return m_member;
    }
    template <typename Tp>
    constexpr void set(Tp&& p) {
        static_assert(SameAsCvRef<Tp, T>);
        m_member = move(p);
    }
    template <size_t N, typename _>
    constexpr void set(T&& f) {
        static_assert(N == 0);
        m_member = move(f);
    }
};
}    // namespace ARLib
// tuple_size and tuple_element specializations for ARLib::Tuple
template <typename... Types>
struct std::tuple_size<ARLib::Tuple<Types...>> : ARLib::IntegralConstant<ARLib::size_t, sizeof...(Types)> {};
template <typename... Types>
struct std::tuple_size<const ARLib::Tuple<Types...>> : ARLib::IntegralConstant<ARLib::size_t, sizeof...(Types)> {};
template <class Head, class... Tail>
struct std::tuple_element<0, ARLib::Tuple<Head, Tail...>> {
    using type = Head;
    using BaseType = ARLib::Tuple<Head, Tail...>;
};
template <std::size_t I, class Head, class... Tail>
struct std::tuple_element<I, ARLib::Tuple<Head, Tail...>> : std::tuple_element<I - 1, ARLib::Tuple<Tail...>> {};
template <class Head, class... Tail>
struct std::tuple_element<0, const ARLib::Tuple<Head, Tail...>> {
    using type     = ARLib::AddConstT<Head>;
    using BaseType = ARLib::AddConstT<ARLib::Tuple<Head, Tail...>>;
};
template <std::size_t I, class Head, class... Tail>
struct std::tuple_element<I, const ARLib::Tuple<Head, Tail...>> : std::tuple_element<I - 1, const ARLib::Tuple<Tail...>> {};
namespace ARLib {
template <typename... Args>
auto make_tuple(Args&&... args) {
    return Tuple{ Forward<Args>(args)... };
}
// free functions to avoid template keyword when calling member functions in templated functions.
template <typename Tp, typename... Args>
requires IsAnyOfV<Tp, Args...>
auto& get(Tuple<Args...>& tuple) {
    return tuple.template get<Tp>();
}
template <typename Tp, typename... Args>
requires IsAnyOfV<Tp, Args...>
auto& get(const Tuple<Args...>& tuple) {
    return tuple.template get<Tp>();
}
template <size_t Idx, typename... Args>
requires(Idx < sizeof...(Args))
constexpr auto& get(Tuple<Args...>& tuple) {
    using Base = typename std::tuple_element<Idx, Tuple<Args...>>::BaseType;
    return static_cast<Base&>(tuple).template get<0>();
}
template <size_t Idx, typename... Args>
requires(Idx < sizeof...(Args))
constexpr const auto& get(const Tuple<Args...>& tuple) {
    using Base = typename std::tuple_element<Idx, Tuple<Args...>>::BaseType;
    return static_cast<const Base&>(tuple).template get<0>();
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
    template <size_t... Idxs>
    void _append_all_args(String& str, IndexSequence<Idxs...>) const {
        (str.append(print_conditional(get<Idxs>(m_tuple)) + ", "_s), ...);
    }
    String repr() const {
        String str{ "{ " };
        _append_all_args(str, IndexSequenceFor<Args...>{});
        str = str.substring(0, str.size() - 2);
        str.append(" }");
        return str;
    }
};
}    // namespace ARLib

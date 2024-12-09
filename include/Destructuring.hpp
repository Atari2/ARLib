#pragma once
#include "Utility.hpp"
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
template <typename T, typename U>
struct Pair;
template <typename T, typename... Args>
class Tuple;
template <typename Idx, typename T>
auto get();
}

template <typename T, typename U>
struct std::tuple_size<ARLib::Pair<T, U>> : ARLib::IntegralConstant<ARLib::size_t, 2> {};
template <typename T, typename U>
struct std::tuple_element<0, ARLib::Pair<T, U>> {
    using type = T;
};
template <typename T, typename U>
struct std::tuple_element<1, ARLib::Pair<T, U>> {
    using type = U;
};
template <typename T, typename U>
struct std::tuple_element<0, const ARLib::Pair<T, U>> {
    using type = ARLib::AddConstT<T>;
};
template <typename T, typename U>
struct std::tuple_element<1, const ARLib::Pair<T, U>> {
    using type = ARLib::AddConstT<U>;
};

// tuple_size and tuple_element specializations for ARLib::Tuple
template <typename... Types>
struct std::tuple_size<ARLib::Tuple<Types...>> : ARLib::IntegralConstant<ARLib::size_t, sizeof...(Types)> {};
template <typename... Types>
struct std::tuple_size<const ARLib::Tuple<Types...>> : ARLib::IntegralConstant<ARLib::size_t, sizeof...(Types)> {};
template <class Head, class... Tail>
struct std::tuple_element<0, ARLib::Tuple<Head, Tail...>> {
    using type     = Head;
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
struct std::tuple_element<I, const ARLib::Tuple<Head, Tail...>> :
    std::tuple_element<I - 1, const ARLib::Tuple<Tail...>> {};

namespace ARLib {
template <size_t Idx, typename T>
concept SupportsMemberGet = requires(T& t) {
    { t.template get<Idx>() } -> NonVoid;
};
template <size_t Idx, typename T>
concept SupportsFreeGet = requires(T& t) {
    { ARLib::get<Idx>(t) } -> NonVoid;
};

template <typename T>
concept IsTupleLikeDestructurable = requires(T& t) {
    typename std::tuple_element<0, T>::type;                // specializes std::tuple_element
    { std::tuple_size<T>::value } -> SameAs<size_t>;        // specializes std::tuple_size
} && (SupportsFreeGet<0, T> || SupportsMemberGet<0, T>);    // check if the tuple has a get<0> function

template <typename T>
constexpr size_t recursive_tuple_size();
template <typename T, size_t... Ns>
constexpr size_t recursive_tuple_size(ARLib::IndexSequence<Ns...>) {
    return (recursive_tuple_size<typename std::tuple_element<Ns, T>::type>() + ...);
}
template <typename T>
constexpr size_t recursive_tuple_size() {
    if constexpr (requires { std::tuple_size<T>::value; }) {
        return recursive_tuple_size<T>(ARLib::MakeIndexSequence<std::tuple_size<T>::value>());
    } else {
        return 1;
    }
}
template <size_t N, size_t Start = 0, typename T, size_t PlaceHolder = 0>
decltype(auto) recursive_tuple_element(T& tup) {
    if constexpr (!requires { std::tuple_size<T>::value; }) {
        static_assert(N == 0);
        return tup;
    } else {
        constexpr size_t sz = recursive_tuple_size<typename std::tuple_element<Start, T>::type>();
        if constexpr (N < sz) {
            if constexpr (SupportsMemberGet<Start, T>) {
                return recursive_tuple_element<N>(tup.template get<Start>());
            } else {
                return recursive_tuple_element<N>(ARLib::get<Start>(tup));
            }
        } else {
            return recursive_tuple_element<N - sz, Start + 1, T, sz>(tup);
        }
    }
}
template <typename T>
struct FlattenTuple {
    T& m_tuple;
    template <size_t N>
    auto& get() {
        return recursive_tuple_element<N>(m_tuple);
    }
};
template <typename T>
FlattenTuple(T) -> FlattenTuple<T>;
template <typename T>
constexpr FlattenTuple<T> flatten_tuple(T& tup) {
    return FlattenTuple<T>{ tup };
}
}    // namespace ARLib
template <typename T>
struct std::tuple_size<ARLib::FlattenTuple<T>> {
    constexpr static size_t value = ARLib::recursive_tuple_size<T>();
};
template <size_t N, typename T>
struct std::tuple_element<N, ARLib::FlattenTuple<T>> {
    using type = decltype(ARLib::recursive_tuple_element<N>(ARLib::declval<T&>()));
};

#pragma once
#include "Concepts.h"
// we love UB
// forward declaring things in std:: is UB
// but this avoids having to #include <utility>
// so I'll do it
namespace std {
template <class T>
struct tuple_size;
template <size_t I, class T>
struct tuple_element;
}    // namespace std
namespace ARLib {
template <typename T, typename... Args>
class Tuple;
template <typename A, typename B>
struct Pair;

template <template <typename...> typename Cls, typename... Args>
constexpr bool is_tuple(const Cls<Args...>*) {
    struct DerivedClassBase {};
    using DerivedClass = Tuple<DerivedClassBase, Args...>;
    if constexpr (DerivedFrom<DerivedClass, Cls<Args...>>) {
        return true;
    } else {
        return false;
    }
}
template <typename Cls>
constexpr bool is_tuple(const Cls*) {
    return false;
}
template <template <typename, typename> typename Cls, typename A, typename B>
constexpr bool is_pair(const Cls<A, B>*) {
    using ThisType = RemoveCvRefT<Cls<A, B>>;
    using PairCls = Pair<A, B>;
    if constexpr (SameAs<ThisType, PairCls>) {
        return true;
    } else {
        return false;
    }
}
template <typename Cls>
constexpr bool is_pair(const Cls*) {
    return false;
}
template <typename T>
constexpr bool is_tuple_or_pair_base() {
    constexpr const RemoveReferenceT<T>* base_ptr{};
    return is_tuple(base_ptr) || is_pair(base_ptr);
}
template <typename T>
constexpr inline bool IsTupleV = is_tuple_or_pair_base<T>();

template <size_t Accum, typename T, typename... Types>
struct TupleSizeImpl;

template <size_t Accum, typename T, typename... Types>
struct TupleSizeImplInner {
    constexpr static inline size_t Size  = IsTupleV<T> ? TupleSizeImpl<0, T>::value : 1;
    constexpr static inline size_t value = TupleSizeImpl<Accum + Size, Types...>::value;
};
template <size_t Accum, template <typename...> typename T, typename... Args, typename... Types>
struct TupleSizeImplInner<Accum, T<Args...>, Types...> {
    constexpr static inline size_t Size  = IsTupleV<T<Args...>> ? TupleSizeImpl<0, Args...>::value : 1;
    constexpr static inline size_t value = TupleSizeImpl<Accum + Size, Types...>::value;
};
template <size_t Accum, typename T>
struct TupleSizeImplInner<Accum, T> {
    constexpr static inline size_t value = Accum + 1;
};
template <size_t Accum, template <typename...> typename T, typename... Args>
struct TupleSizeImplInner<Accum, T<Args...>> {
    constexpr static inline size_t Size  = IsTupleV<T<Args...>> ? TupleSizeImpl<0, Args...>::value : 1;
    constexpr static inline size_t value = Accum + Size;
};
template <size_t Accum, typename T, typename... Types>
struct TupleSizeImpl {
    constexpr static inline size_t value = TupleSizeImplInner<Accum, RemoveCvRefT<T>, RemoveCvRefT<Types>...>::value;
};
}    // namespace ARLib
// tuple_size and tuple_element specializations for ARLib::Tuple
template <typename... Types>
struct std::tuple_size<ARLib::Tuple<Types...>> : ARLib::TupleSizeImpl<0, Types...> {};
template <typename... Types>
struct std::tuple_size<const ARLib::Tuple<Types...>> : ARLib::TupleSizeImpl<0, Types...> {};

template <typename A, typename B>
struct std::tuple_size<ARLib::Pair<A, B>> : ARLib::TupleSizeImpl<0, A, B> {};
template <typename A, typename B>
struct std::tuple_size<const ARLib::Pair<A, B>> : ARLib::TupleSizeImpl<0, A, B> {};

template <std::size_t I, class Head, class... Tail>
struct std::tuple_element<I, ARLib::Tuple<Head, Tail...>> {
    using type     = decltype(ARLib::declval<ARLib::Tuple<Head, Tail...>>().template get<I>());
    using BaseType = ARLib::Tuple<Head, Tail...>;
};
template <std::size_t I, template <typename...> typename Head, class... Args, class... Tail>
requires ARLib::IsTupleV<Head<Args...>>
struct std::tuple_element<I, ARLib::Tuple<Head<Args...>, Tail...>> {
    using type     = decltype(ARLib::declval<ARLib::Tuple<Head<Args...>, Tail...>>().template get<I>());
    using BaseType = ARLib::Tuple<Head<Args...>, Tail...>;
};

template <std::size_t I, typename A, typename B>
struct std::tuple_element<I, ARLib::Pair<A, B>> {
    using type     = decltype(ARLib::declval<ARLib::Pair<A, B>>().template get<I>());
    using BaseType = ARLib::Pair<A, B>;
};
template <std::size_t I, template <typename...> typename Head, class... Args, typename B>
requires ARLib::IsTupleV<Head<Args...>>
struct std::tuple_element<I, ARLib::Pair<Head<Args...>, B>> {
    using type     = decltype(ARLib::declval<ARLib::Pair<Head<Args...>, B>>().template get<I>());
    using BaseType = ARLib::Pair<Head<Args...>, B>;
};


template <std::size_t I, class Head, class... Tail>
struct std::tuple_element<I, const ARLib::Tuple<Head, Tail...>> {
    using type = decltype(ARLib::declval<const ARLib::Tuple<Head, Tail...>>().template get<I>());
    using BaseType = ARLib::AddConstT<ARLib::Tuple<Head, Tail...>>;
};
template <std::size_t I, template <typename...> typename Head, class... Args, class... Tail>
requires ARLib::IsTupleV<Head<Args...>>
struct std::tuple_element<I, const ARLib::Tuple<Head<Args...>, Tail...>> {
    using type = decltype(ARLib::declval<const ARLib::Tuple<Head<Args...>, Tail...>>().template get<I>());
    using BaseType = ARLib::AddConstT<ARLib::Tuple<Head<Args...>, Tail...>>;
};
template <std::size_t I, typename A, typename B>
struct std::tuple_element<I, const ARLib::Pair<A, B>> {
    using type     = decltype(ARLib::declval<const ARLib::Pair<A, B>>().template get<I>());
    using BaseType = ARLib::AddConstT<ARLib::Pair<A, B>>;
};
template <std::size_t I, template <typename...> typename Head, class... Args, typename B>
requires ARLib::IsTupleV<Head<Args...>>
struct std::tuple_element<I, const ARLib::Pair<Head<Args...>, B>> {
    using type     = decltype(ARLib::declval<const ARLib::Pair<Head<Args...>, B>>().template get<I>());
    using BaseType = ARLib::AddConstT<ARLib::Pair<Head<Args...>, B>>;
};
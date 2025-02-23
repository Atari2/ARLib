#pragma once
#if USE_NEW_TUPLE
    #include "Concepts.hpp"
namespace ARLib {
// namespace v2 {
template <typename T, size_t Idx, bool CanBeEmpty = IsEmptyV<T> && !IsFinalV<T>>
struct CompressedTupleElem {
    T elem;
    template <typename U>
    requires(!SameAs<CompressedTupleElem, DecayT<U>>)
    constexpr explicit CompressedTupleElem(U&& u) : elem{ Forward<U>(u) } {}
    constexpr auto& get() & noexcept { return elem; }
    constexpr const auto& get() const& noexcept { return elem; }
    constexpr auto get() && noexcept { return move(elem); }
    void set(T&& value) { elem = move(value); }
    constexpr bool operator==(const CompressedTupleElem& other) const
    requires EqualityComparable<T>
    {
        return elem == other.elem;
    }
};
template <typename T, size_t Idx>
struct CompressedTupleElem<T, Idx, true> : private T {
    template <typename U>
    requires(!SameAs<CompressedTupleElem, DecayT<U>>)
    constexpr explicit CompressedTupleElem(U&& u) : T{ Forward<U>(u) } {}
    constexpr T& get() & noexcept { return *this; }
    constexpr const T& get() const& noexcept { return *this; }
    constexpr T get() && noexcept { return move(*this); }
    void set(T&& value) { *this = move(value); }
    constexpr bool operator==(const CompressedTupleElem& other) const
    requires EqualityComparable<T>
    {
        return static_cast<T&>(*this) == static_cast<const T&>(other);
    }
};
template <typename... Types, size_t... Idxs>
consteval auto& tuple_impl(IndexSequence<Idxs...>) {
    struct TupleImpl : CompressedTupleElem<Types, Idxs>... {
        constexpr TupleImpl(Types&&... args) : CompressedTupleElem<Types, Idxs>{ Forward<Types>(args) }... {}
    };
    TupleImpl* ptr{ nullptr };
    return *ptr;
}
template <typename... Types>
requires(sizeof...(Types) > 0)
struct Tuple : public RemoveCvRefT<decltype(tuple_impl<Types...>(IndexSequenceFor<Types...>{}))> {
    using Base = RemoveCvRefT<decltype(tuple_impl<Types...>(IndexSequenceFor<Types...>{}))>;

    using TupleArray = IdentityTypeArray<Types...>;

    template <size_t Idx>
    using NthElem = CompressedTupleElem<typename TupleArray::template At<Idx>, Idx>;

    constexpr static size_t size = sizeof...(Types);

    constexpr Tuple(const Tuple&)                = default;
    constexpr Tuple(Tuple&&) noexcept            = default;
    constexpr Tuple& operator=(const Tuple&)     = default;
    constexpr Tuple& operator=(Tuple&&) noexcept = default;
    constexpr Tuple(Types&&... args) : Base{ Forward<Types>(args)... } {}
    template <typename... Args>
    requires(sizeof...(Args) == sizeof...(Types))
    constexpr Tuple(Args&&... args) : Base{ Types{ args }... } {}
    template <size_t... Idxs>
    constexpr bool equality_impl(const Tuple& other, IndexSequence<Idxs...>) const {
        return ((static_cast<const NthElem<Idxs>&>(*this) == static_cast<const NthElem<Idxs>&>(other)) && ...);
    }
    constexpr bool operator==(const Tuple& other) const
    requires(EqualityComparable<Types> && ...)
    {
        return equality_impl(other, IndexSequenceFor<Types...>{});
    }
    template <typename T>
    constexpr auto& operator=(T&& value) {
        set(Forward<T>(value));
        return *this;
    }
    template <size_t Idx>
    constexpr const auto& get() const& {
        return static_cast<const NthElem<Idx>*>(this)->get();
    }
    template <size_t Idx>
    constexpr auto& get() & {
        return static_cast<NthElem<Idx>*>(this)->get();
    }
    template <size_t Idx>
    constexpr auto get() && {
        return static_cast<NthElem<Idx>&&>(*this).get();
    }
    template <typename T>
    constexpr const auto& get() const& {
        return get<TupleArray::template IndexOf<T>>();
    }
    template <typename T>
    constexpr auto& get() & {
        return get<TupleArray::template IndexOf<T>>();
    }
    template <typename T>
    constexpr auto&& get() && {
        return get<TupleArray::template IndexOf<T>>();
    }
    template <size_t Idx, typename T>
    void set(T&& value) {
        static_cast<NthElem<Idx>&>(*this).set(Forward<T>(value));
    }
    template <typename T>
    void set(T&& value) {
        constexpr static auto Index = TupleArray::template IndexOf<T>;
        static_cast<NthElem<Index>&>(*this).set(Forward<T>(value));
    }
    auto flatten() const& { return flatten_tuple(*this); }
    auto flatten() & { return flatten_tuple(*this); }
    auto flatten() && { return flatten_tuple(move(*this)); }
};
template <typename... Args>
Tuple(Args&&...) -> Tuple<Args...>;
// }    // namespace v2

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
    return static_cast<Base&>(tuple).get();
}
template <size_t Idx, typename... Args>
requires(Idx < sizeof...(Args))
constexpr const auto& get(const Tuple<Args...>& tuple) {
    using Base = typename std::tuple_element<Idx, Tuple<Args...>>::BaseType;
    return static_cast<const Base&>(tuple).get();
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
template <typename... Types>
struct std::tuple_size<ARLib::Tuple<Types...>> : std::integral_constant<size_t, sizeof...(Types)> {};
template <typename... Types>
struct std::tuple_size<const ARLib::Tuple<Types...>> : std::integral_constant<size_t, sizeof...(Types)> {};
template <ARLib::size_t Idx, typename... Types>
struct std::tuple_element<Idx, ARLib::Tuple<Types...>> {
    using TupleArray = ARLib::IdentityTypeArray<Types...>;
    using BaseType   = ARLib::Tuple<Types...>::template NthElem<Idx>;
    using type       = typename TupleArray::template At<Idx>;
};
template <ARLib::size_t Idx, typename... Types>
struct std::tuple_element<Idx, const ARLib::Tuple<Types...>> {
    using TupleArray = ARLib::IdentityTypeArray<Types...>;
    using BaseType   = ARLib::AddConstT<typename ARLib::Tuple<Types...>::template NthElem<Idx>>;
    using type       = ARLib::AddConstT<typename TupleArray::template At<Idx>>;
};
template <typename... Types>
struct std::tuple_size<ARLib::Tuple<Types...>&> : std::integral_constant<size_t, sizeof...(Types)> {};
template <typename... Types>
struct std::tuple_size<const ARLib::Tuple<Types...>&> : std::integral_constant<size_t, sizeof...(Types)> {};
template <ARLib::size_t Idx, typename... Types>
struct std::tuple_element<Idx, ARLib::Tuple<Types...>&> {
    using TupleArray = ARLib::IdentityTypeArray<Types...>;
    using BaseType   = ARLib::Tuple<Types...>::template NthElem<Idx>;
    using type       = typename TupleArray::template At<Idx>;
};
template <ARLib::size_t Idx, typename... Types>
struct std::tuple_element<Idx, const ARLib::Tuple<Types...>&> {
    using TupleArray = ARLib::IdentityTypeArray<Types...>;
    using BaseType   = ARLib::AddConstT<ARLib::AddLvalueReferenceT<typename ARLib::Tuple<Types...>::template NthElem<Idx>>>;
    using type       = ARLib::AddConstT<ARLib::AddLvalueReferenceT<typename TupleArray::template At<Idx>>>;
};
#endif
#pragma once
#include "Algorithm.h"
#include "Concepts.h"
#include "Iterator.h"
#include "AdvancedIterators.h"
#include "TypeTraits.h"
namespace ARLib {
template <typename Cont>
size_t container_size(const Cont& cont) {
    if constexpr (EnumerableC<Cont>) {
        if constexpr (IterCanSubtractForSize<decltype(declval<Cont>().begin())>) {
            return cont.end() - cont.begin();
        } else {
            return cont.size();
        }
    } else if constexpr (CanKnowSize<Cont>) {
        return cont.size();
    } else {
        static_assert(AlwaysFalse<Cont>, "Cannot get size of this container");
    }
}
template <typename It>
concept IterCanAdvanceWithOffset = requires(It it, size_t dist) {
                                       { it + dist } -> SameAs<It>;
                                   };
template <typename It>
auto advance_iterator(It iter, It end, size_t dist) {
    if constexpr (IterCanAdvanceWithOffset<It>) {
        return iter + static_cast<int>(dist);
    } else if constexpr (Incrementable<It>) {
        for (size_t i = 0; i < dist && iter != end; i++) ++iter;
        return iter;
    }
}
template <typename T, ComparatorType CMP = ComparatorType::NotEqual, typename = EnableIfT<IsNonboolIntegral<T>>>
class Iterate {
    T m_begin;
    T m_end;
    T m_step;

    public:
    Iterate(T begin, T end, T step = T{ 1 }) : m_begin(begin), m_end(end), m_step(step) {
        T diff = end - begin;
        HARD_ASSERT_FMT(
        !(diff < T{ 0 } && step > T{ 0 }),
        "This loop that starts from %d and arrives to %d with step %d will never stop", begin, end, step
        )
        if constexpr (CMP == ComparatorType::NotEqual || CMP == ComparatorType::Equal) {
            HARD_ASSERT_FMT(
            ((diff % step) == 0),
            "This loop that starts from %d and arrives to %d with step %d will never stop because "
            "the comparator type is checking for strict equality or inequality and the gap between "
            "begin and end is not divisible by the step",
            begin, end, step
            )
        }
    }
    auto begin() const { return LoopIterator<T, CMP>{ m_begin, m_step }; }
    auto end() const { return LoopIterator<T, CMP>{ m_end, m_step }; }
};
template <EnumerableC T>
class Enumerate {
    T m_container;

    public:
    explicit Enumerate(T container) : m_container(Forward<T>(container)) {}
    auto begin() const { return Enumerator{ ARLib::begin(m_container), 0ull }; }
    auto end() const { return Enumerator{ ARLib::end(m_container), m_container.size() }; }
};
template <EnumerableC T>
Enumerate(T&) -> Enumerate<T&>;
template <EnumerableC T>
Enumerate(T&&) -> Enumerate<T>;
template <EnumerableC T>
auto enumerate(T&& cont) {
    return Enumerate{ Forward<T>(cont) };
}
template <EnumerableC T>
auto enumerate(T& cont) {
    return Enumerate{ cont };
}
template <EnumerableC T>
class ConstEnumerate {
    using TRef = AddConstT<typename RemoveReference<T>::type>&;
    TRef m_container;

    public:
    explicit ConstEnumerate(const T& container) : m_container(container) {}
    auto begin() const { return ConstEnumerator{ ARLib::begin(m_container), 0ull }; }
    auto end() const { return ConstEnumerator{ ARLib::end(m_container), m_container.size() }; }
};
// simple iterator pair wrapper, will only iterate until the shorter of the 2 iterators has elements.
template <EnumerableC F, EnumerableC S>
class PairIterate {
    F& m_first;
    S& m_second;
    size_t m_size;

    public:
    PairIterate(F& f, S& s) : m_first(f), m_second(s) { m_size = min_bt(container_size(f), container_size(s)); }
    auto begin() const { return PairIterator{ m_first.begin(), m_second.begin() }; }
    auto end() const { return PairIterator{ m_first.end(), m_second.end() }; }
};
template <Iterable... Conts>
class ZipIterate {
    using ZipContainer = decltype(types_into_ref_tuple<Conts...>());
    ZipContainer m_tuple;
    size_t m_size;
    template <size_t... Vals>
    auto ibegin(IndexSequence<Vals...>) const {
        return ZipIterator{ m_tuple.template get<Vals>().begin()... };
    }
    template <size_t... Vals>
    auto iend(IndexSequence<Vals...>) const {
        return ZipIterator{
            advance_iterator(m_tuple.template get<Vals>().begin(), m_tuple.template get<Vals>().end(), m_size)...
        };
    }
    template <size_t... Vals>
    size_t isize(IndexSequence<Vals...>) const {
        size_t sizes[sizeof...(Conts)]{ container_size(m_tuple.template get<Vals>())... };
        return *min(ARLib::begin(sizes), ARLib::end(sizes));
    }
    public:
    ZipIterate(Conts&... conts) : m_tuple{ conts... }, m_size{} { m_size = isize(IndexSequenceFor<Conts...>{}); }
    auto begin() const { return ibegin(IndexSequenceFor<Conts...>{}); }
    auto end() const { return iend(IndexSequenceFor<Conts...>{}); }
    size_t size() const { return m_size; }
};
template <Iterable... Conts>
auto zip(Conts&... conts) {
    return ZipIterate{ conts... };
}
template <Iterable Container, typename Functor>
class FilterIterate {
    using Iter = decltype(declval<Container>().begin());
    Iter m_start;
    Iter m_end;
    Functor m_func;
    public:
    FilterIterate(Container& cont, Functor func) : m_start(cont.begin()), m_end(cont.end()), m_func(func) {}
    FilterIterate(Iter start, Iter end, Functor func) : m_start(start), m_end(end), m_func(func) {}
    auto begin() const { return IfIterator<Iter, Functor>{ m_start, m_end, m_func }; }
    auto end() const { return IfIterator<Iter, Functor>{ m_end, m_end, m_func, true }; }
};
template <Iterable Container, typename Functor>
class MapIterate {
    using Iter = decltype(declval<Container>().begin());
    Iter m_start;
    Iter m_end;
    Functor m_func;

    public:
    MapIterate(Container& cont, Functor func) : m_start(cont.begin()), m_end(cont.end()), m_func(func) {}
    MapIterate(Iter start, Iter end, Functor func) : m_start(start), m_end(end), m_func(func) {}
    auto begin() const { return MapIterator<Iter, Functor>{ m_start, m_end, m_func }; }
    auto end() const { return MapIterator<Iter, Functor>{ m_end, m_end, m_func }; }
};
template <typename Container>
concept ReverseIterable = requires(Container cont) {
                              { cont.rbegin() } -> IteratorConcept;
                              { cont.rend() } -> IteratorConcept;
                          };
template <ReverseIterable Container>
class ReverseIterate {
    using Iter = decltype(declval<Container>().rbegin());
    static_assert(IteratorConcept<Iter>, "ReverseIterate requires that Iter matches the Iterator concept");
    Iter m_start;
    Iter m_end;

    public:
    ReverseIterate(Container& cont) : m_start(cont.rbegin()), m_end(cont.rend()) {}
    ReverseIterate(Iter start, Iter end) : m_start(start), m_end(end) {}
    auto begin() const { return m_start; }
    auto end() const { return m_end; }
};
template <Iterable Containter>
auto reverse(Containter& cont) {
    return ReverseIterate{ cont };
}
}    // namespace ARLib

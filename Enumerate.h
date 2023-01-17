#pragma once
#include "Algorithm.h"
#include "Iterator.h"
#include "Pair.h"
namespace ARLib {
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
    using TRef = typename RemoveReference<T>::type&;
    TRef m_container;

    public:
    explicit Enumerate(T& container) : m_container(container) {}
    auto begin() const { return Enumerator{ ARLib::begin(m_container), 0ull }; }
    auto end() const { return Enumerator{ ARLib::end(m_container), m_container.size() }; }
};
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
template <Iterable F, Iterable S>
class PairIterate {
    F& m_first;
    S& m_second;

    public:
    PairIterate(F& f, S& s) : m_first(f), m_second(s) {}
    auto begin() const { return PairIterator{ m_first.begin(), m_second.begin() }; }
    auto end() const { return PairIterator{ m_first.end(), m_second.end() }; }
};
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
}    // namespace ARLib

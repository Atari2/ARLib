#pragma once
#include "BaseTraits.hpp"
#include "Tuple.hpp"
#include "TypeTraits.hpp"
#include "Pair.hpp"
#include "Concepts.hpp"
namespace ARLib {
template <typename F, typename S>
class PairIterator {
    using IterUnit = Pair<F, S>;
    IterUnit m_current_pair;
    using FT = decltype(*m_current_pair.template get<0>());
    using ST = decltype(*m_current_pair.template get<1>());

    public:
    PairIterator(F first, S second) : m_current_pair(first, second) {}
    explicit PairIterator(IterUnit curr_pair) : m_current_pair(curr_pair){};
    Pair<FT, ST> operator*() { return { *m_current_pair.template get<0>(), *m_current_pair.template get<1>() }; }
    PairIterator& operator++() {
        m_current_pair.template get<0>()++;
        m_current_pair.template get<1>()++;
        return *this;
    }
    PairIterator operator++(int) {
        auto copy = *this;
        this->operator++();
        return copy;
    }
    PairIterator& operator--() {
        m_current_pair.template get<0>()--;
        m_current_pair.template get<1>()--;
        return *this;
    }
    PairIterator operator--(int) {
        auto copy = *this;
        this->operator--();
        return copy;
    }
    bool operator==(const PairIterator& other) const { return m_current_pair == other.m_current_pair; }
    bool operator!=(const PairIterator& other) const { return m_current_pair != other.m_current_pair; }
    bool operator<(const PairIterator& other) {
        return m_current_pair.template get<0>() < other.m_current_pair.template get<0>() &&
               m_current_pair.template get<1>() < other.m_current_pair.template get<1>();
    }
    bool operator>(const PairIterator& other) {
        return m_current_pair.template get<0>() > other.m_current_pair.template get<0>() &&
               m_current_pair.template get<1>() > other.m_current_pair.template get<1>();
    }
};
template <typename Tp, typename... Tps>
auto& types_into_ref_tuple(Tp&& tp, Tps&&... tps) {
    using RealT = decltype(tp);
    using Ref   = ConditionalT<IsLvalueReferenceV<RealT>, Tp&, Tp>;
    if constexpr (sizeof...(Tps) == 0) {
        Tuple<Ref>* tup{};
        return *tup;
    } else {
        using Arr = TypeArray<Ref>;
        return types_into_ref_tuple<Tps...>(Arr{}, Forward<Tps>(tps)...);
    }
}
template <typename Tp, typename... Tps, typename... Rem>
auto& types_into_ref_tuple(TypeArray<Tps...>, Tp&& tp, Rem&&... rem) {
    using RealT = decltype(tp);
    using Ref   = ConditionalT<IsLvalueReferenceV<RealT>, Tp&, Tp>;
    if constexpr (sizeof...(Rem) == 0) {
        Tuple<Tps..., Ref>* tup{};
        return *tup;
    } else {
        using Arr = TypeArray<Tps..., Ref>;
        return types_into_ref_tuple<Rem...>(Arr{}, Forward<Rem>(rem)...);
    }
}
template <typename Tp, typename... Tps>
auto& types_into_iter_tuple(Tp tp, Tps... tps) {
    using RealT = decltype(*tp);
    if constexpr (sizeof...(Tps) == 0) {
        Tuple<RealT>* tup{};
        return *tup;
    } else {
        using Arr = TypeArray<RealT>;
        return types_into_iter_tuple<Tps...>(Arr{}, tps...);
    }
}
template <typename Tp, typename... Tps, typename... Rem>
auto& types_into_iter_tuple(TypeArray<Tps...>, Tp tp, Rem... rem) {
    using RealT = decltype(*tp);
    if constexpr (sizeof...(Rem) == 0) {
        Tuple<Tps..., RealT>* tup{};
        return *tup;
    } else {
        using Arr = TypeArray<Tps..., RealT>;
        return types_into_iter_tuple<Rem...>(Arr{}, rem...);
    }
}
template <typename... Tps>
class ZipIterator {
    using IterUnit = Tuple<Tps...>;
    IterUnit m_current_tuple;
    template <size_t... Vals>
    void iincrement(IndexSequence<Vals...>) {
        (..., m_current_tuple.template get<Vals>()++);
    }
    template <size_t... Vals>
    void idecrement(IndexSequence<Vals...>) {
        (..., m_current_tuple.template get<Vals>()--);
    }
    template <size_t... Vals>
    auto iget(IndexSequence<Vals...>) {
        using TupleType = RemoveReferenceT<decltype(types_into_iter_tuple(m_current_tuple.template get<Vals>()...))>;
        return TupleType{ *m_current_tuple.template get<Vals>()... };
    }

    public:
    ZipIterator(Tps... iterators) : m_current_tuple(iterators...) {}
    explicit ZipIterator(IterUnit curr_pair) : m_current_tuple(curr_pair){};
    auto operator*() { return iget(IndexSequenceFor<Tps...>{}); }
    auto operator*() const { return iget(IndexSequenceFor<Tps...>{}); }
    ZipIterator& operator++() {
        iincrement(IndexSequenceFor<Tps...>{});
        return *this;
    }
    ZipIterator operator++(int) {
        auto copy = *this;
        this->operator++();
        return copy;
    }
    ZipIterator& operator--() {
        idecrement(IndexSequenceFor<Tps...>{});
        return *this;
    }
    ZipIterator operator--(int) {
        auto copy = *this;
        this->operator--();
        return copy;
    }
    bool operator==(const ZipIterator& other) const { return m_current_tuple == other.m_current_tuple; }
    bool operator!=(const ZipIterator& other) const { return m_current_tuple != other.m_current_tuple; }
    bool operator<(const ZipIterator& other) {
        return (... && (m_current_tuple.template get<Tps>() < other.m_current_tuple.template get<Tps>()));
    }
    bool operator>(const ZipIterator& other) {
        return (... && (m_current_tuple.template get<Tps>() > other.m_current_tuple.template get<Tps>()));
    }
};
template <Incrementable Iter>
requires(Dereferencable<Iter>)
class Enumerator {
    using T  = decltype(*declval<Iter>());
    using Rt = RemoveReferenceT<T>;
    Iter m_iter;
    size_t m_index;

    using Unit = Pair<size_t, T>;

    public:
    using InputValueType  = IteratorInputType<Iter>;
    using OutputValueType = Unit;
    explicit Enumerator(Rt* begin) : m_iter(begin), m_index(0) {}
    Enumerator(Rt* begin, size_t index) : m_iter(begin), m_index(index) {}
    Enumerator(Iter iter, size_t index) : m_iter(iter), m_index(index) {}
    Unit operator*() { return { m_index, *m_iter }; }
    Enumerator& operator++() {
        m_index++;
        m_iter++;
        return *this;
    }
    Enumerator operator++(int) {
        auto copy = *this;
        this->operator++();
        return copy;
    }
    bool operator==(const Enumerator& other) const { return m_iter == other.m_iter; }
    bool operator!=(const Enumerator& other) const { return m_iter != other.m_iter; }
    bool operator<(const Enumerator& other) const { return m_iter < other.m_iter; }
    bool operator>(const Enumerator& other) const { return m_iter > other.m_iter; }
    size_t operator-(const Enumerator& other) const { return m_iter - other.m_iter; }
};
template <typename T>
class ConstEnumerator {
    ConstIterator<T> m_iter;
    size_t m_index;

    using Unit = Pair<size_t, const T&>;

    public:
    explicit ConstEnumerator(const T* begin) : m_iter(begin), m_index(0) {}
    ConstEnumerator(const T* begin, size_t index) : m_iter(begin), m_index(index) {}
    ConstEnumerator(ConstIterator<T> iter, size_t index) : m_iter(iter), m_index(index) {}
    Unit operator*() { return { m_index, *m_iter }; }
    ConstEnumerator& operator++() {
        m_index++;
        m_iter++;
        return *this;
    }
    ConstEnumerator operator++(int) {
        auto copy = *this;
        this->operator++();
        return copy;
    }
    bool operator==(const ConstEnumerator& other) const { return m_index == other.m_index; }
    bool operator!=(const ConstEnumerator& other) const { return m_index != other.m_index; }
    bool operator<(const ConstEnumerator& other) const { return m_index < other.m_index; }
    bool operator>(const ConstEnumerator& other) const { return m_index > other.m_index; }
    size_t operator-(const ConstEnumerator& other) const { return m_iter - other.m_iter; }
};
}    // namespace ARLib

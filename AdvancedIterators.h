#pragma once
#include "BaseTraits.h"
#include "Tuple.h"
#include "TypeTraits.h"
#include "Pair.h"
#include "Concepts.h"

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
template <typename... Tps>
requires(sizeof...(Tps) > 0)
auto types_into_ref_tuple() {
    if constexpr (sizeof...(Tps) == 1) {
        Tuple<Tps&...>* ptr{};
        return *ptr;
    } else {
        using A1  = TypeArray<Tps...>;
        using Cur = typename A1::template At<0>;
        using A2  = TypeArray<AddLvalueReferenceT<Cur>>;
        return types_into_ref_tuple<1>(A1{}, A2{});
    }
}
template <size_t Idx, typename... P1, typename... P2>
requires(Idx < sizeof...(P1))
auto types_into_ref_tuple(TypeArray<P1...>, TypeArray<P2...>) {
    using A1  = TypeArray<P1...>;
    using Cur = typename A1::template At<Idx>;
    using A2  = TypeArray<P2..., AddLvalueReferenceT<Cur>>;
    if constexpr ((Idx + 1) == sizeof...(P1)) {
        Tuple<P2..., AddLvalueReferenceT<Cur>>* ptr{};
        return *ptr;
    } else {
        return types_into_ref_tuple<Idx + 1>(A1{}, A2{});
    }
}
template <typename... Tps>
requires(sizeof...(Tps) > 0)
auto types_into_iter_tuple() {
    if constexpr (sizeof...(Tps) == 1) {
        using VT = AddLvalueReferenceT<decltype(*declval<Tps...>())>;
        using TupleT = Tuple<VT>;
        TupleT* ptr{};
        return *ptr;
    } else {
        using A1  = TypeArray<Tps...>;
        using Cur = typename A1::template At<0>;
        using VT = AddLvalueReferenceT<decltype(*declval<Cur>())>;
        using A2  = TypeArray<VT>;
        return types_into_iter_tuple<1>(A1{}, A2{});
    }
}
template <size_t Idx, typename... P1, typename... P2>
requires(Idx < sizeof...(P1))
auto types_into_iter_tuple(TypeArray<P1...>, TypeArray<P2...>) {
    using A1  = TypeArray<P1...>;
    using Cur = typename A1::template At<Idx>;
    using VT = AddLvalueReferenceT<decltype(*declval<Cur>())>;
    using A2  = TypeArray<P2..., VT>;
    if constexpr ((Idx + 1) == sizeof...(P1)) {
        using TupleT = Tuple<P2..., VT>;
        TupleT* ptr{};
        return *ptr;
    } else {
        return types_into_iter_tuple<Idx + 1>(A1{}, A2{});
    }
}
template <typename... Tps>
class ZipIterator {
    using RetVal = decltype(types_into_iter_tuple<Tps...>());
    using IterUnit = Tuple<Tps...>;
    IterUnit m_current_pair;
    public:
    ZipIterator(Tps... iterators) : m_current_pair(iterators...) {}
    explicit ZipIterator(IterUnit curr_pair) : m_current_pair(curr_pair) {};
    RetVal operator*() { return RetVal{ *m_current_pair.template get<Tps>()... }; }
    ZipIterator& operator++() {
        (..., m_current_pair.template get<Tps>()++);
        return *this;
    }
    ZipIterator operator++(int) {
        auto copy = *this;
        this->operator++();
        return copy;
    }
    ZipIterator& operator--() {
        (..., m_current_pair.template get<Tps>()--);
        return *this;
    }
    ZipIterator operator--(int) {
        auto copy = *this;
        this->operator--();
        return copy;
    }
    bool operator==(const ZipIterator& other) const { return m_current_pair == other.m_current_pair; }
    bool operator!=(const ZipIterator& other) const { return m_current_pair != other.m_current_pair; }
    bool operator<(const ZipIterator& other) {
        return (... && (m_current_pair.template get<Tps>() < other.m_current_pair.template get<Tps>()));
    }
    bool operator>(const ZipIterator& other) {
        return (... && (m_current_pair.template get<Tps>() > other.m_current_pair.template get<Tps>()));
    }
};
template <IteratorConcept Iter>
class Enumerator {
    using T  = decltype(*declval<Iter>());
    using Rt = RemoveReferenceT<T>;
    Iter m_iter;
    size_t m_index;

    using Unit = Pair<size_t, T>;

    public:
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
    bool operator==(const Enumerator& other) const { return m_index == other.m_index; }
    bool operator!=(const Enumerator& other) const { return m_index != other.m_index; }
    bool operator<(const Enumerator& other) const { return m_index < other.m_index; }
    bool operator>(const Enumerator& other) const { return m_index > other.m_index; }
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
}

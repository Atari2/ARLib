#pragma once
#include "BaseTraits.h"
#include "Comparator.h"
#include "Concepts.h"
#include "TypeTraits.h"
#include "Types.h"
#include "IteratorInspection.h"
#include "Invoke.h"
namespace ARLib {

constexpr static size_t it_npos = static_cast<size_t>(-1);
template <typename T>
class IteratorBase {
    protected:
    T* m_current;
    constexpr explicit IteratorBase(T* ptr) : m_current(ptr) {}
    constexpr IteratorBase(const IteratorBase<T>& other) : m_current(other.m_current) {}
    constexpr IteratorBase(IteratorBase<T>&& other) noexcept : m_current(other.m_current) { other.m_current = nullptr; }

    public:
    using ValueType = T;
    constexpr bool operator==(const IteratorBase<T>& other) const { return m_current == other.m_current; }
    constexpr bool operator!=(const IteratorBase<T>& other) const { return m_current != other.m_current; }
    constexpr bool operator<(const IteratorBase<T>& other) { return m_current < other.m_current; }
    constexpr bool operator>(const IteratorBase<T>& other) { return m_current > other.m_current; }
};
// for some godforsaken reason
#define m_current IteratorBase<T>::m_current
template <typename T>
class Iterator final : public IteratorBase<T> {
    public:
    explicit Iterator(T* start) : IteratorBase<T>(start) {}
    Iterator(const Iterator<T>& other) : IteratorBase<T>(other) {}
    Iterator(Iterator<T>&& other) noexcept : IteratorBase<T>(other) {}
    T& operator*() { return *m_current; }
    Iterator& operator=(const Iterator<T>& other) {
        m_current = other.m_current;
        return *this;
    }
    Iterator& operator=(Iterator<T>&& other) noexcept {
        m_current       = other.m_current;
        other.m_current = nullptr;
        return *this;
    }
    Iterator<T>& operator++() {
        m_current++;
        return *this;
    }
    Iterator<T> operator++(int) {
        auto copy = *this;
        m_current++;
        return copy;
    }
    Iterator<T>& operator+=(int offset) {
        m_current += offset;
        return *this;
    }
    Iterator<T> operator+(int offset) { return Iterator<T>{ m_current + offset }; }
    Iterator<T>& operator--() {
        m_current--;
        return *this;
    }
    Iterator<T> operator--(int) {
        auto copy = *this;
        m_current--;
        return copy;
    }
    Iterator<T>& operator-=(int offset) {
        m_current -= offset;
        return *this;
    }
    Iterator<T> operator-(int offset) { return Iterator<T>{ m_current - offset }; }
    size_t operator-(const Iterator<T>& other) const { return static_cast<size_t>(m_current - other.m_current); }
};
template <typename Ct>
class ConstIterator final : public IteratorBase<typename AddConst<Ct>::type> {

    public:
    using T = AddConstT<Ct>;
    constexpr explicit ConstIterator(T* start) : IteratorBase<T>(start) {}
    constexpr ConstIterator(const ConstIterator<Ct>& other) : IteratorBase<T>(other) {}
    constexpr ConstIterator(ConstIterator<Ct>&& other) noexcept : IteratorBase<T>(other) { other.m_current = nullptr; }
    constexpr ConstIterator<Ct>& operator=(const ConstIterator<Ct>& other) {
        m_current = other.m_current;
        return *this;
    }
    constexpr ConstIterator<Ct>& operator=(ConstIterator<Ct>&& other) noexcept {
        m_current       = other.m_current;
        other.m_current = nullptr;
        return *this;
    }
    constexpr const T& operator*() const { return *m_current; }
    constexpr ConstIterator<Ct>& operator++() {
        m_current++;
        return *this;
    }
    constexpr ConstIterator<Ct> operator++(int) {
        auto copy = *this;
        m_current++;
        return copy;
    }
    constexpr ConstIterator<Ct>& operator+=(int offset) {
        m_current += offset;
        return *this;
    }
    constexpr ConstIterator<Ct> operator+(int offset) { return ConstIterator<Ct>{ m_current + offset }; }
    constexpr ConstIterator<Ct>& operator--() {
        m_current--;
        return *this;
    }
    constexpr ConstIterator<Ct> operator--(int) {
        auto copy = *this;
        m_current--;
        return copy;
    }
    constexpr ConstIterator<Ct>& operator-=(int offset) {
        m_current -= offset;
        return *this;
    }
    constexpr ConstIterator<Ct> operator-(int offset) { return ConstIterator<Ct>{ m_current - offset }; }
    constexpr size_t operator-(const ConstIterator<Ct>& other) const {
        return static_cast<size_t>(m_current - other.m_current);
    }
};
template <typename T>
class ReverseIterator final : public IteratorBase<T> {
    public:
    explicit ReverseIterator(T* end) : IteratorBase<T>(end) {}
    ReverseIterator(const ReverseIterator<T>& other) : IteratorBase<T>(other.m_current) {}
    ReverseIterator(ReverseIterator<T>&& other) noexcept : IteratorBase<T>(other) { other.m_current = nullptr; }
    T& operator*() { return *m_current; }
    ReverseIterator& operator=(const ReverseIterator<T>& other) { m_current = other.m_current; }
    ReverseIterator& operator=(ReverseIterator<T>&& other) noexcept {
        m_current       = other.m_current;
        other.m_current = nullptr;
    }
    ReverseIterator<T>& operator++() {
        m_current--;
        return *this;
    }
    ReverseIterator<T> operator++(int) {
        auto copy = *this;
        m_current--;
        return copy;
    }
    ReverseIterator<T>& operator+=(int offset) {
        m_current -= offset;
        return *this;
    }
    ReverseIterator<T> operator+(int offset) { return ReverseIterator<T>{ m_current - offset }; }
    ReverseIterator<T>& operator--() {
        m_current++;
        return *this;
    }
    ReverseIterator<T> operator--(int) {
        auto copy = *this;
        m_current++;
        return copy;
    }
    ReverseIterator<T>& operator-=(int offset) {
        m_current += offset;
        return *this;
    }
    ReverseIterator<T> operator-(int offset) { return { m_current + offset }; }
};
template <typename Ct>
class ConstReverseIterator final : public IteratorBase<typename AddConst<Ct>::type> {
    using T = typename AddConst<Ct>::type;

    public:
    explicit ConstReverseIterator(T* end) : IteratorBase<T>(end) {}
    ConstReverseIterator(const ConstReverseIterator<Ct>& other) : IteratorBase<T>(other.m_current) {}
    ConstReverseIterator(ConstReverseIterator<Ct>&& other) noexcept : IteratorBase<T>(other) {
        other.m_current = nullptr;
    }
    const T& operator*() { return *m_current; }
    ConstReverseIterator<Ct>& operator=(const ConstReverseIterator<Ct>& other) { m_current = other.m_current; }
    ConstReverseIterator<Ct>& operator=(ConstReverseIterator<Ct>&& other) noexcept {
        m_current       = other.m_current;
        other.m_current = nullptr;
    }
    ConstReverseIterator<Ct>& operator++() {
        m_current--;
        return *this;
    }
    ConstReverseIterator<Ct> operator++(int) {
        auto copy = *this;
        m_current--;
        return copy;
    }
    ConstReverseIterator<Ct>& operator+=(int offset) {
        m_current -= offset;
        return *this;
    }
    ConstReverseIterator<Ct> operator+(int offset) { return ConstReverseIterator<Ct>{ m_current - offset }; }
    ConstReverseIterator<Ct>& operator--() {
        m_current++;
        return *this;
    }
    ConstReverseIterator<Ct> operator--(int) {
        auto copy = *this;
        m_current++;
        return copy;
    }
    ConstReverseIterator<Ct>& operator-=(int offset) {
        m_current += offset;
        return *this;
    }
    ConstReverseIterator<Ct> operator-(int offset) { return { m_current + offset }; }
};
#undef m_current
template <typename T, ComparatorType CMP, typename = EnableIfT<IsNonboolIntegral<T>>>
class LoopIterator {
    T m_current;
    T m_step;
    Comparator<T, CMP> m_cmp{};

    public:
    LoopIterator(T current, T step) : m_current(current), m_step(step) {}
    LoopIterator(const LoopIterator& other)                = default;
    LoopIterator(LoopIterator&& other) noexcept            = default;
    LoopIterator& operator=(LoopIterator&& other) noexcept = default;
    LoopIterator& operator=(const LoopIterator& other)     = default;
    T operator*() { return m_current; }
    LoopIterator& operator++() {
        m_current += m_step;
        return *this;
    }
    LoopIterator operator++(int) {
        auto copy = *this;
        m_current += m_step;
        return copy;
    }
    bool operator==(const LoopIterator& other) const { return m_current == other.m_current; }
    bool operator!=(const LoopIterator& other) const { return m_cmp.compare(m_current, other.m_current); }
    bool operator<(const LoopIterator& other) { return m_current < other.m_current; }
    bool operator>(const LoopIterator& other) { return m_current > other.m_current; }
    size_t operator-(const LoopIterator& other) {
        return static_cast<size_t>(m_current) - static_cast<size_t>(other.m_current);
    }
};
template <class IterUnit, class Functor>
requires requires(IterUnit iter, Functor func) {
             { invoke(func, *iter) } -> SameAs<bool>;
         }
class IfIterator {
    IterUnit m_current_iter;
    IterUnit m_end;
    Functor m_func;
    void advance() {
        if (m_current_iter == m_end) return;
        while (!invoke(m_func, *m_current_iter)) {
            ++m_current_iter;
            if (m_current_iter == m_end) return;
        }
    }

    public:
    using InputValueType  = IteratorInputType<IterUnit>;
    using OutputValueType = IteratorOutputType<IterUnit>;
    IfIterator(IterUnit unit, IterUnit end, Functor func, bool is_end = false) :
        m_current_iter(unit), m_end(end), m_func(func) {
        if (!is_end) { advance(); }
    }
    OutputValueType operator*() { return *m_current_iter; }
    const OutputValueType operator*() const { return *m_current_iter; }
    IfIterator& operator++() {
        if (m_current_iter == m_end) return *this;
        ++m_current_iter;
        advance();
        return *this;
    }
    IfIterator operator++(int) {
        IfIterator iter{ *this };
        this->operator++();
        return iter;
    }
    bool operator==(const IfIterator& other) const { return m_current_iter == other.m_current_iter; }
    bool operator!=(const IfIterator& other) const { return m_current_iter != other.m_current_iter; }
    bool operator<(const IfIterator& other) { return m_current_iter < other.m_current_iter; }
    bool operator>(const IfIterator& other) { return m_current_iter > other.m_current_iter; }
    size_t operator-(const IfIterator& other) const {
        if (other.m_end != m_end) return it_npos;
        return m_current_iter - other.m_current_iter;
    }
};

template <class ItemType, class Functor>
using MapType =
ConditionalT<IsConstV<ItemType>, AddConstT<InvokeResultT<Functor, ItemType>>, InvokeResultT<Functor, ItemType>>;
template <class IterUnit, class Functor>
requires requires(IterUnit iter, Functor func) { invoke(func, *iter); }
class MapIterator {
    IterUnit m_current_iter;
    IterUnit m_end;
    Functor m_func;

    public:
    using InputValueType  = IteratorInputType<IterUnit>;
    using OutputValueType = MapType<IteratorOutputType<IterUnit>, Functor>;
    MapIterator(IterUnit unit, IterUnit end, Functor func) : m_current_iter(unit), m_end(end), m_func(func) {}
    OutputValueType operator*() { return invoke(m_func, *m_current_iter); }
    const OutputValueType operator*() const { return invoke(m_func, *m_current_iter); }
    MapIterator& operator++() {
        if (m_current_iter == m_end) return *this;
        ++m_current_iter;
        return *this;
    }
    MapIterator operator++(int) {
        MapIterator iter{ *this };
        this->operator++();
        return iter;
    }
    MapIterator& operator--() {
        --m_current_iter;
        return *this;
    }
    MapIterator operator--(int) {
        MapIterator iter{ *this };
        this->operator--();
        return iter;
    }
    bool operator==(const MapIterator& other) const { return m_current_iter == other.m_current_iter; }
    bool operator!=(const MapIterator& other) const { return m_current_iter != other.m_current_iter; }
    bool operator<(const MapIterator& other) { return m_current_iter < other.m_current_iter; }
    bool operator>(const MapIterator& other) { return m_current_iter > other.m_current_iter; }
    size_t operator-(const MapIterator& other) const
    requires IterCanSubtractForSize<IterUnit>
    {
        if (other.m_end != m_end) return it_npos;
        return m_current_iter - other.m_current_iter;
    }
};
}    // namespace ARLib

#pragma once
#include "Comparator.h"
#include "Concepts.h"
#include "Pair.h"
#include "TypeTraits.h"
#include "Types.h"

namespace ARLib {

    template <typename T>
    struct IteratorType {
        using Type = T;
    };

    template <typename Iter>
    class IteratorOperators {
        public:
        virtual bool operator==(const Iter&) const = 0;
        virtual bool operator!=(const Iter&) const = 0;
        virtual bool operator<(const Iter&) = 0;
        virtual bool operator>(const Iter&) = 0;

        virtual ~IteratorOperators() = default;
    };

    template <typename T>
    class IteratorBase : public IteratorOperators<IteratorBase<T>> {
        protected:
        T* m_current;
        explicit IteratorBase(T* ptr) : m_current(ptr) {}

        IteratorBase(const IteratorBase<T>& other) : m_current(other.m_current) {}

        IteratorBase(IteratorBase<T>&& other) noexcept : m_current(other.m_current) { other.m_current = nullptr; }

        public:
        using Type = T;
        bool operator==(const IteratorBase<T>& other) const override { return m_current == other.m_current; }
        bool operator!=(const IteratorBase<T>& other) const override { return m_current != other.m_current; }
        bool operator<(const IteratorBase<T>& other) override { return m_current < other.m_current; }
        bool operator>(const IteratorBase<T>& other) override { return m_current > other.m_current; }
        virtual ~IteratorBase() = default;
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

        Iterator& operator=(Iterator<T>&& other)  noexcept {
            m_current = other.m_current;
            other.m_current = nullptr;
            return *this;
        }

        Iterator<T>& operator++() {
            m_current++;
            return *this;
        }

        Iterator<T> operator++(int) { return Iterator<T>{m_current++}; }

        Iterator<T>& operator+=(int offset) {
            m_current += offset;
            return *this;
        }

        Iterator<T> operator+(int offset) { return Iterator<T>{m_current + offset}; }

        Iterator<T>& operator--() {
            m_current--;
            return *this;
        }

        Iterator<T> operator--(int) { return Iterator<T>{m_current--}; }

        Iterator<T>& operator-=(int offset) {
            m_current -= offset;
            return *this;
        }

        Iterator<T> operator-(int offset) { return Iterator<T>{m_current - offset}; }
        size_t operator-(const Iterator<T>& other) { return static_cast<size_t>(m_current - other.m_current); }
    };

    template <typename Ct>
    class ConstIterator final : public IteratorBase<typename AddConst<Ct>::type> {
        using T = typename AddConst<Ct>::type;

        public:
        explicit ConstIterator(T* start) : IteratorBase<T>(start) {}

        ConstIterator(const ConstIterator<Ct>& other) : IteratorBase<T>(other) {}

        ConstIterator(ConstIterator<Ct>&& other) noexcept : IteratorBase<T>(other) { other.m_current = nullptr; }

        ConstIterator<Ct>& operator=(const ConstIterator<Ct>& other) { m_current = other.m_current; }

        ConstIterator<Ct>& operator=(ConstIterator<Ct>&& other)  noexcept {
            m_current = other.m_current;
            other.m_current = nullptr;
        }

        const T& operator*() { return *m_current; }

        ConstIterator<Ct>& operator++() {
            m_current++;
            return *this;
        }

        ConstIterator<Ct> operator++(int) { return {m_current++}; }

        ConstIterator<Ct>& operator+=(int offset) {
            m_current += offset;
            return *this;
        }

        ConstIterator<Ct> operator+(int offset) { return {m_current + offset}; }

        ConstIterator<Ct>& operator--() {
            m_current--;
            return *this;
        }

        ConstIterator<Ct> operator--(int) { return {m_current--}; }

        ConstIterator<Ct>& operator-=(int offset) {
            m_current -= offset;
            return *this;
        }

        ConstIterator<Ct> operator-(int offset) { return {m_current - offset}; }
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
            m_current = other.m_current;
            other.m_current = nullptr;
        }

        ReverseIterator<T>& operator++() {
            m_current--;
            return *this;
        }

        ReverseIterator<T> operator++(int) { return {m_current--}; }

        ReverseIterator<T>& operator+=(int offset) {
            m_current -= offset;
            return *this;
        }

        ReverseIterator<T> operator+(int offset) { return {m_current - offset}; }

        ReverseIterator<T>& operator--() {
            m_current++;
            return *this;
        }

        ReverseIterator<T> operator--(int) { return {m_current++}; }

        ReverseIterator<T>& operator-=(int offset) {
            m_current += offset;
            return *this;
        }

        ReverseIterator<T> operator-(int offset) { return {m_current + offset}; }
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
            m_current = other.m_current;
            other.m_current = nullptr;
        }

        ConstReverseIterator<Ct>& operator++() {
            m_current--;
            return *this;
        }

        ConstReverseIterator<Ct> operator++(int) { return {m_current--}; }

        ConstReverseIterator<Ct>& operator+=(int offset) {
            m_current -= offset;
            return *this;
        }

        ConstReverseIterator<Ct> operator+(int offset) { return {m_current - offset}; }

        ConstReverseIterator<Ct>& operator--() {
            m_current++;
            return *this;
        }

        ConstReverseIterator<Ct> operator--(int) { return {m_current++}; }

        ConstReverseIterator<Ct>& operator-=(int offset) {
            m_current += offset;
            return *this;
        }

        ConstReverseIterator<Ct> operator-(int offset) { return {m_current + offset}; }
    };
#undef m_current

    template <typename T, ComparatorType CMP, typename = EnableIfT<IsNonboolIntegral<T>>>
    class LoopIterator : public IteratorOperators<LoopIterator<T, CMP>>, IteratorType<T> {
        T m_current;
        T m_step;
        Comparator<T, CMP> m_cmp{};

        public:
        LoopIterator(T current, T step) : m_current(current), m_step(step) {}
        LoopIterator(const LoopIterator& other) = default;
        LoopIterator(LoopIterator&& other) noexcept = default;
        LoopIterator& operator=(LoopIterator&& other) noexcept = default;
        LoopIterator& operator=(const LoopIterator& other) = default;

        T operator*() { return m_current; }
        LoopIterator& operator++() {
            m_current += m_step;
            return *this;
        }
        LoopIterator operator++(int) { return {m_current + m_step, m_step}; }
        bool operator==(const LoopIterator& other) const override { return m_current == other.m_current; }
        bool operator!=(const LoopIterator& other) const override { return m_cmp.compare(m_current, other.m_current); }
        bool operator<(const LoopIterator& other) override { return m_current < other.m_current; }
        bool operator>(const LoopIterator& other) override { return m_current > other.m_current; }
        size_t operator-(const LoopIterator& other) {
            return static_cast<size_t>(m_current) - static_cast<size_t>(other.m_current);
        }
    };

    template <typename T>
    class Enumerator : public IteratorOperators<Enumerator<T>>, IteratorType<T> {
        Iterator<T> m_iter;
        size_t m_index;

        using Unit = Pair<size_t, T&>;

        public:
        explicit Enumerator(T* begin) : m_iter(begin), m_index(0) {}
        Enumerator(T* begin, size_t index) : m_iter(begin), m_index(index) {}
        Enumerator(Iterator<T> iter, size_t index) : m_iter(iter), m_index(index) {}
        Unit operator*() { return {m_index, *m_iter}; }

        Enumerator& operator++() {
            m_index++;
            m_iter++;
            return *this;
        }

        Enumerator operator++(int) { return {m_iter++, m_index++}; }
        bool operator==(const Enumerator& other) const override { return m_index == other.m_index; }
        bool operator!=(const Enumerator& other) const override { return m_index != other.m_index; }
        bool operator<(const Enumerator& other) override { return m_index < other.m_index; }
        bool operator>(const Enumerator& other) override { return m_index > other.m_index; }
        size_t operator-(const Enumerator& other) { return m_iter - other.m_iter; }
    };

    template <typename F, typename S>
    class PairIterator {
        using IterUnit = Pair<F, S>;
        IterUnit m_current_pair;
        using FT = decltype(*m_current_pair.template get<0>());
        using ST = decltype(*m_current_pair.template get<1>());

        public:
        PairIterator(F first, S second) : m_current_pair(first, second) {}
        explicit PairIterator(IterUnit curr_pair) : m_current_pair(curr_pair){};
        Pair<FT, ST> operator*() { return {*m_current_pair.template get<0>(), *m_current_pair.template get<1>()}; }
        PairIterator& operator++() {
            m_current_pair.template get<0>()++;
            m_current_pair.template get<1>()++;
            return *this;
        }

        PairIterator operator++(int) {
            return {m_current_pair.template get<0>()++, m_current_pair.template get<1>()++};
        }
        bool operator==(const PairIterator& other) const {
            return m_current_pair.template get<0>() == other.m_current_pair.template get<0>() &&
                   m_current_pair.template get<1>() == other.m_current_pair.template get<1>();
        }
        bool operator!=(const PairIterator& other) const {
            return m_current_pair.template get<0>() != other.m_current_pair.template get<0>() &&
                   m_current_pair.template get<1>() != other.m_current_pair.template get<1>();
        }
        bool operator<(const PairIterator& other) {
            return m_current_pair.template get<0>() < other.m_current_pair.template get<0>() &&
                   m_current_pair.template get<1>() < other.m_current_pair.template get<1>();
        }
        bool operator>(const PairIterator& other) {
            return m_current_pair.template get<0>() > other.m_current_pair.template get<0>() &&
                   m_current_pair.template get<1>() > other.m_current_pair.template get<1>();
        }
    };

    template <class T, class Functor>
    requires CallableWithRes<Functor, bool, const T&>
    class IfIterator {
        using IterUnit = Iterator<T>;
        IterUnit m_current_iter;
        IterUnit m_end;
        Functor m_func;

        void advance() {
            if (m_current_iter == m_end) return;
            while (!m_func(*m_current_iter)) {
                ++m_current_iter;
                if (m_current_iter == m_end) return;
            }
        }

        public:
        IfIterator(IterUnit unit, IterUnit end, Functor func, bool is_end = false) :
            m_current_iter(unit), m_end(end), m_func(func) {
            if (!is_end) { advance(); }
        }
        T& operator*() { return *m_current_iter; }
        IfIterator& operator++() {
            if (m_current_iter == m_end) return *this;
            ++m_current_iter;
            advance();
            return *this;
        }

        IfIterator operator++(int) {
            IfIterator iter{*this};
            ++iter;
            return iter;
        }
        bool operator==(const IfIterator& other) const { return m_current_iter == other.m_current_iter; }
        bool operator!=(const IfIterator& other) const { return m_current_iter != other.m_current_iter; }
        bool operator<(const IfIterator& other) { return m_current_iter < other.m_current_iter; }
        bool operator>(const IfIterator& other) { return m_current_iter > other.m_current_iter; }
    };
} // namespace ARLib

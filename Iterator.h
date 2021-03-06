#pragma once
#include "Comparator.h"
#include "Concepts.h"
#include "TypeTraits.h"
#include "Types.h"

namespace ARLib {

    static constexpr size_t it_npos = static_cast<size_t>(-1);

    template <typename T>
    struct IteratorType {
        using Type = T;
    };

    template <Iterable T>
    struct IterableTraits {
        using IterType = decltype(declval<T>().begin());
        using Tp = decltype(*declval<IterType>());
        using ItemType = RemoveReferenceT<ConditionalT<IsConstV<T>, AddConstT<Tp>, Tp>>;
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

        Iterator& operator=(Iterator<T>&& other) noexcept {
            m_current = other.m_current;
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

        Iterator<T> operator+(int offset) { return Iterator<T>{m_current + offset}; }

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

        Iterator<T> operator-(int offset) { return Iterator<T>{m_current - offset}; }
        size_t operator-(const Iterator<T>& other) const { return static_cast<size_t>(m_current - other.m_current); }
    };

    template <typename Ct>
    class ConstIterator final : public IteratorBase<typename AddConst<Ct>::type> {
        using T = typename AddConst<Ct>::type;

        public:
        explicit ConstIterator(T* start) : IteratorBase<T>(start) {}

        ConstIterator(const ConstIterator<Ct>& other) : IteratorBase<T>(other) {}

        ConstIterator(ConstIterator<Ct>&& other) noexcept : IteratorBase<T>(other) { other.m_current = nullptr; }

        ConstIterator<Ct>& operator=(const ConstIterator<Ct>& other) {
            m_current = other.m_current;
            return *this;
        }

        ConstIterator<Ct>& operator=(ConstIterator<Ct>&& other) noexcept {
            m_current = other.m_current;
            other.m_current = nullptr;
            return *this;
        }

        const T& operator*() const { return *m_current; }

        ConstIterator<Ct>& operator++() {
            m_current++;
            return *this;
        }

        ConstIterator<Ct> operator++(int) {
            auto copy = *this;
            m_current++;
            return copy;
        }

        ConstIterator<Ct>& operator+=(int offset) {
            m_current += offset;
            return *this;
        }

        ConstIterator<Ct> operator+(int offset) { return ConstIterator<Ct>{m_current + offset}; }

        ConstIterator<Ct>& operator--() {
            m_current--;
            return *this;
        }

        ConstIterator<Ct> operator--(int) {
            auto copy = *this;
            m_current--;
            return copy;
        }

        ConstIterator<Ct>& operator-=(int offset) {
            m_current -= offset;
            return *this;
        }

        ConstIterator<Ct> operator-(int offset) { return ConstIterator<Ct>{m_current - offset}; }
        size_t operator-(const ConstIterator<Ct>& other) const {
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
            m_current = other.m_current;
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

        ReverseIterator<T> operator+(int offset) { return ReverseIterator<T>{m_current - offset}; }

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

        ConstReverseIterator<Ct> operator++(int) {
            auto copy = *this;
            m_current--;
            return copy;
        }

        ConstReverseIterator<Ct>& operator+=(int offset) {
            m_current -= offset;
            return *this;
        }

        ConstReverseIterator<Ct> operator+(int offset) { return ConstReverseIterator<Ct>{m_current - offset}; }

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
        LoopIterator operator++(int) {
            auto copy = *this;
            m_current += m_step;
            return copy;
        }
        bool operator==(const LoopIterator& other) const override { return m_current == other.m_current; }
        bool operator!=(const LoopIterator& other) const override { return m_cmp.compare(m_current, other.m_current); }
        bool operator<(const LoopIterator& other) override { return m_current < other.m_current; }
        bool operator>(const LoopIterator& other) override { return m_current > other.m_current; }
        size_t operator-(const LoopIterator& other) {
            return static_cast<size_t>(m_current) - static_cast<size_t>(other.m_current);
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
} // namespace ARLib

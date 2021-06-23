#pragma once
#include "BaseTraits.h"
#include "Types.h"

namespace ARLib {
    template <typename T>
    class IteratorBase {
        protected:
        T* m_current;
        IteratorBase(T* ptr) : m_current(ptr) {}

        IteratorBase(const IteratorBase<T>& other) : m_current(other.m_current) {}

        IteratorBase(IteratorBase<T>&& other) : m_current(other.m_current) { other.m_current = nullptr; }

        public:
        using Type = T;
        virtual bool operator==(const IteratorBase<T>& other) const { return m_current == other.m_current; }
        virtual bool operator!=(const IteratorBase<T>& other) const { return m_current != other.m_current; }
        virtual bool operator<(const IteratorBase<T>& other) { return m_current < other.m_current; }
        virtual bool operator>(const IteratorBase<T>& other) { return m_current > other.m_current; }
        virtual size_t operator-(const IteratorBase<T>& other) { return m_current - other.m_current; }

        virtual ~IteratorBase() {}
    };

    // for some god forsaken reason
#define m_current IteratorBase<T>::m_current

    template <typename T>
    class Iterator final : public IteratorBase<T> {
        public:
        Iterator(T* start) : IteratorBase<T>(start) {}

        Iterator(const Iterator<T>& other) : IteratorBase<T>(other) {}

        Iterator(Iterator<T>&& other) noexcept : IteratorBase<T>(other) {}

        T& operator*() { return *m_current; }

        Iterator& operator=(const Iterator<T>& other) {
            m_current = other.m_current;
            return *this;
        }

        Iterator& operator=(Iterator<T>&& other) {
            m_current = other.m_current;
            other.m_current = nullptr;
            return *this;
        }

        Iterator<T>& operator++() {
            m_current++;
            return *this;
        }

        Iterator<T> operator++(int) { return {m_current++}; }

        Iterator<T>& operator+=(int offset) {
            m_current += offset;
            return *this;
        }

        Iterator<T> operator+(int offset) { return {m_current + offset}; }

        Iterator<T>& operator--() {
            m_current--;
            return *this;
        }

        Iterator<T> operator--(int) { return {m_current--}; }

        Iterator<T>& operator-=(int offset) {
            m_current -= offset;
            return *this;
        }

        Iterator<T> operator-(int offset) { return {m_current - offset}; }

        bool operator==(const Iterator<T>& other) const { return m_current == other.m_current; }
        bool operator!=(const Iterator<T>& other) const { return m_current != other.m_current; }
        bool operator<(const Iterator<T>& other) { return m_current < other.m_current; }
        bool operator>(const Iterator<T>& other) { return m_current > other.m_current; }
        size_t operator-(const Iterator<T>& other) { return m_current - other.m_current; }
    };

    template <typename Ct>
    class ConstIterator final : public IteratorBase<typename AddConst<Ct>::type> {
        using T = typename AddConst<Ct>::type;

        public:
        ConstIterator(T* start) : IteratorBase<T>(start) {}

        ConstIterator(const ConstIterator<Ct>& other) : IteratorBase<T>(other) {}

        ConstIterator(ConstIterator<Ct>&& other) : IteratorBase<T>(other) { other.m_current = nullptr; }

        ConstIterator<Ct>& operator=(const ConstIterator<Ct>& other) { m_current = other.m_current; }

        ConstIterator<Ct>& operator=(ConstIterator<Ct>&& other) {
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
        ReverseIterator(T* end) : IteratorBase<T>(end) {}

        ReverseIterator(const ReverseIterator<T>& other) : IteratorBase<T>(other.m_current) {}

        ReverseIterator(ReverseIterator<T>&& other) : IteratorBase<T>(other) { other.m_current = nullptr; }

        T& operator*() { return *m_current; }

        ReverseIterator& operator=(const ReverseIterator<T>& other) { m_current = other.m_current; }

        ReverseIterator& operator=(ReverseIterator<T>&& other) {
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
        ConstReverseIterator(T* end) : IteratorBase<T>(end) {}

        ConstReverseIterator(const ConstReverseIterator<Ct>& other) : IteratorBase<T>(other.m_current) {}

        ConstReverseIterator(ConstReverseIterator<Ct>&& other) : IteratorBase<T>(other) { other.m_current = nullptr; }

        const T& operator*() { return *m_current; }

        ConstReverseIterator<Ct>& operator=(const ConstReverseIterator<Ct>& other) { m_current = other.m_current; }

        ConstReverseIterator<Ct>& operator=(ConstReverseIterator<Ct>&& other) {
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
} // namespace ARLib

using ARLib::ConstIterator;
using ARLib::ConstReverseIterator;
using ARLib::Iterator;
using ARLib::ReverseIterator;

#pragma once
#include "Algorithm.h"
#include "Assertion.h"
#include "Compat.h"
#include "Iterator.h"
#include "Ordering.h"
#include "Utility.h"
#include "cstring_compat.h"

namespace ARLib {

    template <typename T>
    struct DefaultOrdering {
        forceinline Ordering operator()(const T& first, const T& second) { return first <=> second; }
    };

    template <typename T, class Ordering = DefaultOrdering<T>>
    class SortedVector {

        using Iter = Iterator<T>;
        using ConstIter = ConstIterator<T>;
        using ReverseIter = ReverseIterator<T>;
        using ConstReverseIter = ConstReverseIterator<T>;

        static constexpr inline auto npos = static_cast<size_t>(-1);
        Ordering m_ordering{};
        T* m_storage = nullptr;
        size_t m_size = 0;
        size_t m_capacity = 0;

        void grow_to_capacity_(size_t capacity) {
            HARD_ASSERT(capacity >= m_size, "Capacity should be bigger or equal than size");
            T* new_storage = new T[capacity];
            if constexpr (IsTriviallyCopiableV<T>) {
                memmove(new_storage, m_storage, sizeof(T) * m_size);
            } else {
                for (size_t i = 0; i < m_size; i++) {
                    new_storage[i] = move(m_storage[i]);
                }
            }
            delete[] m_storage;
            m_storage = new_storage;
            m_capacity = capacity;
        }

        void ensure_capacity_() {
            if (m_size == m_capacity) {
                if constexpr (IsTriviallyCopiableV<T>) {
                    grow_to_capacity_(basic_growth(m_capacity + 1));
                } else {
                    grow_to_capacity_(bit_round_growth(m_capacity + 1));
                }
            }
        }

        size_t find_insert_index_(const T& element) {
            if (m_size == 0) return 0;
            size_t left = 0;
            size_t right = m_size - 1;
            if (m_ordering(element, m_storage[left]) == less) return 0;
            if (m_ordering(element, m_storage[right]) == greater) return m_size;
            while (left <= right) {
                size_t mid = (left + right) / 2;
                auto ord = m_ordering(m_storage[mid], element);
                if (ord == equal)
                    return mid;
                else if (ord == greater)
                    right = mid - 1;
                else
                    left = mid + 1;
            }
            return left;
        }

        void insert_single_element_(T&& element) {
            size_t insert_index = find_insert_index_(element);
            ensure_capacity_();
            if constexpr (IsTriviallyCopiableV<T>) {
                memmove(m_storage + insert_index + 1, m_storage + insert_index, sizeof(T) * (m_size - insert_index));
            } else {
                for (size_t i = m_size; i > insert_index; i--) {
                    m_storage[i] = move(m_storage[i - 1]);
                }
            }
            m_storage[insert_index] = move(element);
            m_size++;
        }

        public:
        SortedVector() = default;
        SortedVector(size_t capacity) : m_capacity(capacity) { m_storage = new T[m_capacity]; }

        template <typename... Args>
        SortedVector(T&& val, Args&&... args) requires AllOfV<T, Args...> {
            grow_to_capacity_(sizeof...(args) + 1);
            insert_single_element_(Forward<T>(val));
            (insert_single_element_(Forward<Args>(args)), ...);
        }

        template <typename Functor>
        void for_each(Functor func) {
            for (const auto& item : *this)
                func(item);
        }

        void remove(size_t index) {
            SOFT_ASSERT_FMT((index < m_size), "Index %lu was out of bounds in vector of size %lu", index, m_size)
            m_storage[index].~T();
            m_size--;
            if constexpr (IsTriviallyCopiableV<T>) {
                memmove(m_storage + index, m_storage + index + 1, sizeof(T) * (m_size - index));
            } else {
                for (size_t i = index; i < m_size; i++)
                    m_storage[i] = move(m_storage[i + 1]);
            }
        }

        void remove(const T& val) { remove(find(val)); }

        size_t find(const T& element) {
            size_t left = 0;
            size_t right = m_size - 1;
            if (m_ordering(element, m_storage[left]) == less || m_ordering(element, m_storage[right]) == greater)
                return npos;
            while (left <= right) {
                size_t mid = (left + right) / 2;
                auto ord = m_ordering(m_storage[mid], element);
                if (ord == equal)
                    return mid;
                else if (ord == greater)
                    right = mid - 1;
                else
                    left = mid + 1;
            }
            return npos;
        }
        void insert(T element) { insert_single_element_(Forward<T>(element)); }
        const T& operator[](size_t index) const { return m_storage[index]; }
        T& operator[](size_t index) { return m_storage[index]; }
        T* data() { return m_storage; }
        void clear() {
            m_size = 0;
            delete[] m_storage;
            m_storage = nullptr;
            m_capacity = 0;
        }
        bool empty() const { return m_size == 0; }
        size_t size() const { return m_size; }
        size_t capacity() const { return m_capacity; }
        const Ordering& ordering() const { return m_ordering; }

        Iter begin() const { return {m_storage}; }

        Iter end() const { return {m_storage + m_size}; }

        ConstIter cbegin() const { return {m_storage}; }

        ConstIter cend() const { return {m_storage + m_size}; }

        ReverseIter rbegin() const { return {m_storage + m_size - 1}; }

        ReverseIter rend() const { return {m_storage - 1}; }

        ConstReverseIter crbegin() const { return {m_storage + m_size - 1}; }

        ConstReverseIter crend() const { return {m_storage - 1}; }

        ~SortedVector() {
            if (m_capacity) delete[] m_storage;
        }
    };
} // namespace ARLib

using ARLib::SortedVector;
#pragma once
#include "Algorithm.h"
#include "Assertion.h"
#include "Concepts.h"
#include "Iterator.h"
#include "cstring_compat.h"
#include "std_includes.h"

namespace ARLib {
    // this in origin was `template <MoveAssignable T>`
    // however that failed to compile when used as a return type, didn't when using auto
    // reverted back to static_assert().
    // https://godbolt.org/z/hnYj99MPc
    template <typename T>
    class Vector {
        static_assert(MoveAssignableV<T>);
        using Iter = Iterator<T>;
        using ConstIter = ConstIterator<T>;
        using ReverseIter = ReverseIterator<T>;
        using ConstReverseIter = ConstReverseIterator<T>;

        T* m_storage = nullptr;
        T* m_end_of_storage = nullptr;
        size_t m_capacity = 0;
        size_t m_size = 0;

        void append_internal_single_(T&& value) { m_storage[m_size++] = move(value); }

        void ensure_capacity_() {
            if (m_size == m_capacity) { round_to_capacity_(m_capacity); }
        }

        template <typename... Values>
        void append_internal_(T&& value, Values&&... values) {
            if constexpr (sizeof...(values) == 0) {
                append_internal_single_(Forward<T>(value));
            } else {
                append_internal_single_(Forward<T>(value));
                append_internal_(Forward<Values>(values)...);
            }
        }

        void clear_() {
            delete[] m_storage;
            m_storage = nullptr;
            m_end_of_storage = nullptr;
            m_size = 0;
            m_capacity = 0;
        }

        void shrink_to_size_() {
            m_capacity = m_size;
            resize_to_capacity_(m_size);
        }

        void round_to_capacity_(size_t capacity) {
            if constexpr (IsTriviallyCopiableV<T>) {
                resize_to_capacity_(basic_growth(capacity));
            } else {
                resize_to_capacity_(bit_round_growth(capacity));
            }
        }

        void resize_to_capacity_(size_t capacity) {
            m_capacity = capacity;
            T* new_storage = new T[m_capacity];
            if constexpr (IsTriviallyCopiableV<T>) {
                memcpy(new_storage, m_storage, sizeof(T) * m_size);
            } else {
                for (size_t i = 0; i < m_size; i++) {
                    new_storage[i] = move(m_storage[i]);
                }
            }
            delete[] m_storage;
            m_storage = new_storage;
            m_end_of_storage = m_storage + m_capacity;
        }

        constexpr bool assert_size_(size_t index) const { return index < m_size; }

        public:
        Vector() = default;
        Vector(Vector&& other) noexcept {
            m_storage = other.m_storage;
            m_end_of_storage = other.m_end_of_storage;
            m_capacity = other.m_capacity;
            m_size = other.m_size;
            other.m_storage = nullptr;
            other.m_end_of_storage = nullptr;
            other.m_size = 0;
            other.m_capacity = 0;
        }
        Vector(size_t capacity) { round_to_capacity_(capacity); }

        Vector(std::initializer_list<T> list) {
            round_to_capacity_(list.size());
            size_t i = 0;
            for (T val : list) {
                m_storage[i] = move(val);
                i++;
            }
            m_size = list.size();
        }

        // these 2 first arguments are to force this constructor to be called only when there are 2+ arguments
        template <MoveAssignable... Values>
        Vector(T&& val1, T&& val2, Values&&... values) {
            if constexpr (sizeof...(values) == 0) {
                round_to_capacity_(2);
                append_internal_single_(Forward<T>(val1));
                append_internal_single_(Forward<T>(val2));
                return;
            }
            round_to_capacity_(sizeof...(values) + 2);
            append_internal_(Forward<T>(val1), Forward<T>(val2), Forward<Values>(values)...);
        }

        Vector& operator=(Vector&& other) noexcept {
            m_storage = other.m_storage;
            m_end_of_storage = other.m_end_of_storage;
            m_capacity = other.m_capacity;
            m_size = other.m_size;
            other.m_storage = nullptr;
            other.m_end_of_storage = nullptr;
            other.m_size = 0;
            other.m_capacity = 0;
            return *this;
        }

        void reserve(size_t capacity) {
            if (capacity < m_capacity) return;
            round_to_capacity_(capacity);
        }

        void resize(size_t capacity) requires DefaultConstructible<T> {
            reserve(capacity);
            for (size_t i = 0; i < capacity; i++)
                append({});
        }

        void shrink_to_fit() { shrink_to_size_(); }

        void append(const T& value) {
            ensure_capacity_();
            T val = value;
            append_internal_single_(Forward<T>(val));
        }

        void append(T&& value) {
            ensure_capacity_();
            append_internal_single_(Forward<T>(value));
        }

        void push_back(const T& value) { append(value); }
        void push_back(T&& value) { append(Forward<T>(value)); }

        T&& pop() {
            m_size--;
            return move(m_storage[m_size]);
        }

        T& operator[](size_t index) const {
            SOFT_ASSERT_FMT(assert_size_(index), "Index %lu was out of bounds in vector of size %lu", index, m_size)
            return m_storage[index];
        }

        T& index(size_t index) const {
            SOFT_ASSERT_FMT(assert_size_(index), "Index %lu was out of bounds in vector of size %lu", index, m_size)
            return m_storage[index];
        }

        template <typename Functor>
        void for_each(Functor&& func) {
            for (auto& v : *this)
                func(v);
        }

        void remove_at(size_t index) {
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

        void remove(const T& val) {
            for (size_t i = 0; i < m_size; i++) {
                if (val == m_storage[i]) {
                    remove_at(i);
                    return;
                }
            }
        }

        Iter find(const T& val) {
            for (auto it = begin(); it != end(); it++)
                if (*it == val) return it;
            return end();
        }

        size_t capacity() const { return m_capacity; }

        size_t size() const { return m_size; }

        bool empty() const { return m_size == 0; }

        const T* data() { return m_storage; }

        Iter begin() const { return {m_storage}; }

        Iter end() const { return {m_storage + m_size}; }

        ConstIter cbegin() const { return {m_storage}; }

        ConstIter cend() const { return {m_storage + m_size}; }

        ReverseIter rbegin() const { return {m_storage + m_size - 1}; }

        ReverseIter rend() const { return {m_storage - 1}; }

        ConstReverseIter crbegin() const { return {m_storage + m_size - 1}; }

        ConstReverseIter crend() const { return {m_storage - 1}; }

        T& last() { return m_storage[m_size - 1]; }

        const T& last() const { return m_storage[m_size - 1]; }

        void clear() { clear_(); }

        ~Vector() { clear_(); }
    };

} // namespace ARLib

using ARLib::Vector;

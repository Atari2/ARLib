#pragma once
#include "Assertion.hpp"
#include "Concepts.hpp"
#include "Iterator.hpp"
#include "std_includes.hpp"
namespace ARLib {
template <typename T>
struct SetComparer {
    static_assert(
    EqualityComparable<T>, "Your type must have an equality operator or have a custom SetComprarer "
                           "implementation. SetComprarer just wants a static bool equal(const T&, const T&)"
    );
    static bool equal(const T& first, const T& second) { return first == second; }
};
template <typename T, typename CustomComparer = SetComparer<T>>
class Set {
    using Iter             = Iterator<T>;
    using ReverseIter      = ReverseIterator<T>;
    using ConstIter        = ConstIterator<T>;
    using ConstReverseIter = ConstReverseIterator<T>;
    using Cmp              = CustomComparer;

    size_t m_capacity = 0;
    size_t m_size     = 0;
    T* m_storage      = nullptr;
    void grow_internal_(size_t capacity) {
        HARD_ASSERT_FMT(
        (capacity > m_capacity && capacity > m_size),
        "You can't grow by shrinking the size, size = %lu, attempted capacity = %lu", m_size, capacity
        )
        m_capacity     = capacity;
        T* new_storage = new T[m_capacity];
        for (size_t i = 0; i < m_size; i++) new_storage[i] = move(m_storage[i]);
        delete[] m_storage;
        m_storage = new_storage;
    }
    void check_capacity_() {
        if (m_size == m_capacity) grow_internal_(m_capacity == 0 ? 1 : m_capacity * 2);
    }
    void assert_index_([[maybe_unused]] size_t index) {
        SOFT_ASSERT_FMT((index < m_size), "Index %lu was higher than size %lu", index, m_size);
    }
    T& append_internal_(T&& elem) {
        check_capacity_();
        m_storage[m_size++] = move(elem);
        return m_storage[m_size - 1];
    }
    T& append_internal_(const T& elem) {
        check_capacity_();
        m_storage[m_size++] = elem;
        return m_storage[m_size - 1];
    }
    void clear_() {
        if (m_capacity == 0) return;
        for (size_t i = 0; i < m_size; i++) m_storage[i].~T();
        delete[] m_storage;
        m_storage = nullptr;
    }

    public:
    Set() = default;
    explicit Set(size_t capacity) : m_capacity(capacity), m_storage(new T[capacity]) {}
    template <typename... Args>
    explicit Set(T&& val, Args&&... args)
    requires AllOfV<T, Args...>
    {
        grow_internal_(sizeof...(args) + 1);
        append_internal_(Forward<T>(val));
        (append_internal_(Forward<Args>(args)), ...);
    }
    const T& insert(const T& elem)
    requires CopyAssignable<T>
    {
        ConstIter it = find(elem);
        if (it != cend()) {
            return *it;
        } else {
            return append_internal_(elem);
        }
    }
    const T& insert(T&& elem)
    requires MoveAssignable<T>
    {
        ConstIter it = find(elem);
        if (it != cend()) {
            return *it;
        } else {
            return append_internal_(Forward<T>(elem));
        }
    }
    Iter begin() { return Iter{ m_storage }; }
    Iter end() { return Iter{ m_storage + m_size }; }
    ConstIter begin() const { return ConstIter{ m_storage }; }
    ConstIter end() const { return ConstIter{ m_storage + m_size }; }
    ConstIter cbegin() const { return ConstIter{ m_storage }; }
    ConstIter cend() const { return ConstIter{ m_storage + m_size }; }
    ReverseIter rbegin() const { return ReverseIter{ m_storage + m_size - 1 }; }
    ReverseIter rend() const { return ReverseIter{ m_storage - 1 }; }
    ConstReverseIter crbegin() const { return ConstReverseIter{ m_storage + m_size - 1 }; }
    ConstReverseIter crend() const { return ConstReverseIter{ m_storage - 1 }; }
    bool remove(size_t index) {
        if (index >= m_size) return false;
        return remove(m_storage[index]);
    }
    bool remove(const T& elem) {
        for (size_t i = 0; i < m_size; i++) {
            if (m_storage[i] == elem) {
                m_size--;
                if constexpr (IsTriviallyCopiableV<T>) {
                    m_storage[i].~T();
                    memmove(m_storage + i, m_storage + i + 1, sizeof(T) * (m_size - i));
                } else {
                    for (size_t j = i; j < m_size; j++) { m_storage[j] = move(m_storage[j + 1]); }
                }
                return true;
            }
        }
        return false;
    }
    ConstIter find(const T& elem) const {
        for (size_t i = 0; i < m_size; i++) {
            if (Cmp::equal(m_storage[i], elem)) return ConstIter{ m_storage + i };
        }
        return end();
    }
    bool contains(const T& elem) const { return find(elem) != end(); }
    size_t size() const { return m_size; }
    size_t capacity() const { return m_capacity; }
    void reserve(size_t capacity) { grow_internal_(capacity); }
    T& operator[](size_t index) {
        assert_index_(index);
        return m_storage[index];
    }
    const T& operator[](size_t index) const {
        assert_index_(index);
        return m_storage[index];
    }
    template <typename Functor>
    void for_each(Functor&& func) {
        for (size_t i = 0; i < m_size; i++) func(m_storage[i]);
    }
    void clear() { clear_(); }
    ~Set() {
        if (m_capacity == 0) return;
        delete[] m_storage;
        m_storage = nullptr;
    }
};
}    // namespace ARLib

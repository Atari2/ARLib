#pragma once
#include "Algorithm.hpp"
#include "Allocator.hpp"
#include "Assertion.hpp"
#include "BaseTraits.hpp"
#include "Concepts.hpp"
#include "Iterator.hpp"
#include "Memory.hpp"
#include "PrintInfo.hpp"
#include "RefBox.hpp"
#include "TypeTraits.hpp"
#include "cstring_compat.hpp"
#include "std_includes.hpp"
#include "GenericView.hpp"
#include "Span.hpp"
namespace ARLib {
// this in origin was `template <MoveAssignable T>`
// however that failed to compile when used as a return type, didn't when using auto
// reverted to static_assert().
// https://godbolt.org/z/hnYj99MPc
template <typename T>
class Vector {
    using Iter             = Iterator<T>;
    using ConstIter        = ConstIterator<T>;
    using ReverseIter      = ReverseIterator<T>;
    using ConstReverseIter = ConstReverseIterator<T>;

    T* m_storage        = nullptr;
    T* m_end_of_storage = nullptr;
    size_t m_capacity   = 0;
    size_t m_size       = 0;
    void append_internal_single_(T&& value)
    requires MoveAssignable<T>
    {
        new (&m_storage[m_size++]) T{ ARLib::move(value) };
    }
    void append_internal_single_(const T& value)
    requires CopyAssignable<T>
    {
        new (&m_storage[m_size++]) T{ value };
    }
    void ensure_capacity_() {
        if (m_size == m_capacity) { round_to_capacity_(m_capacity + 1); }
    }
    void clear_() {
        if (m_capacity == 0) return;
        for (size_t i = 0; i < m_size; ++i) { m_storage[i].~T(); }
        deallocate<T, DeallocType::Multiple>(m_storage);
        m_storage        = nullptr;
        m_end_of_storage = nullptr;
        m_size           = 0;
        m_capacity       = 0;
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
        m_capacity     = capacity;
        T* new_storage = allocate_uninitialized<T>(m_capacity);
        if constexpr (MoveConstructibleV<T>) {
            UninitializedMoveConstruct(new_storage, m_storage, m_size);
        } else {
            UninitializedCopyConstruct(new_storage, m_storage, m_size);
        }
        deallocate<T, DeallocType::Multiple>(m_storage);
        m_storage        = new_storage;
        m_end_of_storage = m_storage + m_capacity;
    }
    arlib_forceinline constexpr bool assert_size_(size_t index) const { return index < m_size; }

    public:
    Vector() = default;
    Vector(std::initializer_list<T> ilist) {
        reserve(ilist.size());
        for (const auto& elem : ilist) { append(elem); }
    }
    Vector(T*& storage_ptr, size_t size) :
        m_storage(storage_ptr), m_end_of_storage(storage_ptr + size), m_capacity(size), m_size(size) {
        storage_ptr = nullptr;
    }
    template <IteratorConcept Iter>
    requires Constructible<T, IteratorOutputType<Iter>>
    Vector(Iter begin, Iter end) {
        if constexpr (IterCanSubtractForSize<Iter>) { reserve(end - begin); }
        for (; begin != end; ++begin) { append(T{ *begin }); }
    }
    Vector(Vector&& other) noexcept {
        m_storage              = other.m_storage;
        m_end_of_storage       = other.m_end_of_storage;
        m_capacity             = other.m_capacity;
        m_size                 = other.m_size;
        other.m_storage        = nullptr;
        other.m_end_of_storage = nullptr;
        other.m_size           = 0;
        other.m_capacity       = 0;
    }
    Vector(const Vector& other) noexcept {
        reserve(other.capacity());
        if constexpr (IsTriviallyCopiableV<T>) {
            memcpy(m_storage, other.m_storage, sizeof(T) * other.m_size);
            m_size = other.m_size;
        } else {
            for (auto& val : other) { append(val); }
        }
    }
    Vector& operator=(const Vector& other) {
        if (this == &other) return *this;
        reserve(other.capacity());
        for (size_t i = 0; i < other.m_size; i++) { m_storage[i] = other.m_storage[i]; }
        m_size = other.m_size;
        return *this;
    }
    Vector& operator=(Vector&& other) noexcept {
        deallocate<T, DeallocType::Multiple>(m_storage);
        m_storage              = other.m_storage;
        m_end_of_storage       = other.m_end_of_storage;
        m_capacity             = other.m_capacity;
        m_size                 = other.m_size;
        other.m_storage        = nullptr;
        other.m_end_of_storage = nullptr;
        other.m_size           = 0;
        other.m_capacity       = 0;
        return *this;
    }
    explicit Vector(size_t capacity) { round_to_capacity_(capacity); }
    template <MoveAssignable... Values>
    explicit Vector(T&& val, Values&&... values)
    requires AllOfV<T, Values...>
    {
        reserve(sizeof...(values) + 1);
        append(Forward<T>(val));
        (append(Forward<Values>(values)), ...);
    }
    bool operator==(const Vector& other) const {
        if (size() != other.size()) return false;
        for (size_t i = 0; i < size(); i++) {
            if (m_storage[i] == other[i]) continue;
            return false;
        }
        return true;
    }
    template <typename C>
    requires EqualityComparableWith<C, T> bool
    operator==(const Vector<C>& other) const {
        if (size() != other.size()) return false;
        for (size_t i = 0; i < size(); i++) {
            if (m_storage[i] == other[i]) continue;
            return false;
        }
        return true;
    }
    void reserve(size_t capacity) {
        if (capacity < m_capacity) return;
        round_to_capacity_(capacity);
    }
    void resize(size_t size)
    requires DefaultConstructible<T>
    {
        reserve(size);
        for (size_t i = m_size; i < size; i++) append({});
    }
    template <CallableWithRes<T> Functor>
    void fill(Functor func, size_t count) {
        reserve(count);
        for (size_t i = 0; i < count; i++) append(func());
    }
    template <CallableWithRes<T, T> Functor>
    requires DefaultConstructible<T>
    void fill_pattern(Functor func, size_t count, T start = T{}) {
        reserve(count);
        for (size_t i = 0; i < count; i++) {
            append(start);
            start = move(func(start));
        }
    }
    template <CallableWithRes<T, T> Functor>
    void fill_pattern(Functor func, size_t count, T start) {
        reserve(count);
        for (size_t i = 0; i < count; i++) {
            append(start);
            start = move(func(start));
        }
    }
    void shrink_to_fit() { shrink_to_size_(); }
    void append(const T& value)
    requires CopyAssignable<T>
    {
        ensure_capacity_();
        append_internal_single_(value);
    }
    void append(T&& value)
    requires MoveAssignable<T>
    {
        ensure_capacity_();
        append_internal_single_(Forward<T>(value));
    }
    template <typename... Args>
    requires Constructible<T, Args...>
    void emplace(Args&&... args) {
        ensure_capacity_();
        T val{ Forward<Args>(args)... };
        if constexpr (MoveAssignable<T>) {
            append_internal_single_(Forward<T>(val));
        } else {
            append_internal_single_(val);
        }
    }
    T* release() {
        T* ptr    = m_storage;
        m_storage = nullptr;
        clear_();
        return ptr;
    }
    void insert(size_t index, const T& value)
    requires CopyAssignable<T>
    {
        if constexpr (DefaultConstructible<T>) {
            if (index < m_size) {
                // index is replacing an element
                m_storage[index] = value;
            } else {
                // index is outside of size
                resize(index);
                append(value);
            }
        } else {
            SOFT_ASSERT_FMT(
            assert_size_(index),
            "Index %lu was out of bounds in vector of size %lu and T was not default constructible", index, m_size
            )
            m_storage[index] = value;
        }
    }
    void insert(size_t index, T&& value)
    requires MoveAssignable<T>
    {
        if constexpr (DefaultConstructible<T>) {
            if (index < m_size) {
                // index is replacing an element
                m_storage[index] = move(value);
            } else {
                // index is outside of size
                resize(index);
                append(Forward<T>(value));
            }
        } else {
            SOFT_ASSERT_FMT(
            assert_size_(index),
            "Index %lu was out of bounds in vector of size %lu and T was not default constructible", index, m_size
            )
            m_storage[index] = move(value);
        }
    }
    template <IteratorConcept Iter>
    requires Constructible<T, IteratorOutputType<Iter>>
    void insert(Iter begin, Iter end) {
        if constexpr (IterCanSubtractForSize<Iter>) { reserve(m_size + (end - begin)); }
        for (; begin != end; ++begin) { append(T{ *begin }); }
    }
    void push_back(const T& value)
    requires CopyAssignable<T>
    {
        append(value);
    }
    void push_back(T&& value)
    requires MoveAssignable<T>
    {
        append(Forward<T>(value));
    }
    T pop() {
        m_size--;
        return move(m_storage[m_size]);
    }
    const T& operator[](size_t index) const {
        HARD_ASSERT_FMT(assert_size_(index), "Index %lu was out of bounds in vector of size %lu", index, m_size)
        return m_storage[index];
    }
    T& operator[](size_t index) {
        HARD_ASSERT_FMT(assert_size_(index), "Index %lu was out of bounds in vector of size %lu", index, m_size)
        return m_storage[index];
    }
    const T& index(size_t index) const {
        HARD_ASSERT_FMT(assert_size_(index), "Index %lu was out of bounds in vector of size %lu", index, m_size)
        return m_storage[index];
    }
    const T& index_unchecked(size_t index) const { return m_storage[index]; }
    T& index(size_t index) {
        HARD_ASSERT_FMT(assert_size_(index), "Index %lu was out of bounds in vector of size %lu", index, m_size)
        return m_storage[index];
    }
    T& index_unchecked(size_t index) { return m_storage[index]; }
    template <typename Functor>
    void for_each(Functor&& func) const {
        for (auto& v : *this) func(v);
    }
    template <typename Functor>
    void for_each(Functor&& func) {
        for (auto& v : *this) func(v);
    }
    void for_each(void (*func)(const T&)) const {
        for (const auto& v : *this) func(v);
    }
    void remove_at(size_t index) {
        SOFT_ASSERT_FMT((index < m_size), "Index %lu was out of bounds in vector of size %lu", index, m_size)
        m_storage[index].~T();
        m_size--;
        if constexpr (IsTriviallyCopiableV<T>) {
            memmove(m_storage + index, m_storage + index + 1, sizeof(T) * (m_size - index));
        } else {
            for (size_t i = index; i < m_size; i++) m_storage[i] = move(m_storage[i + 1]);
        }
    }
    Iter remove(Iter it) {
        size_t idx = static_cast<size_t>((&*it) - m_storage);
        remove_at(idx);
        return Iterator{ m_storage + idx };
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
    template <CallableWithRes<bool, const T&> Functor>
    Iter find(Functor func) {
        for (auto it = begin(); it != end(); it++)
            if (func(*it)) return it;
        return end();
    }
    size_t capacity() const { return m_capacity; }
    size_t size() const { return m_size; }
    void set_size(size_t size) {
        resize(size);
        m_size = size;
    }
    auto iter() const& { return IteratorView{ *this }; }
    auto iter() & { return IteratorView{ *this }; }
    auto iter() && {
        auto view  = IteratorView<Vector<T>>{ m_storage, m_size };
        m_capacity = 0;
        return view;
    }
    auto span() const { return Span<const T>{ m_storage, m_size }; }
    auto span() { return Span<T>{ m_storage, m_size }; }
    constexpr auto enumerate() const {
        ConstEnumerate en{ *this };
        return IteratorView{ en };
    }
    constexpr auto enumerate() {
        Enumerate en{ *this };
        return IteratorView{ en };
    }
    bool empty() const { return m_size == 0; }
    const T* data() { return m_storage; }
    Iter begin() { return Iter{ m_storage }; }
    Iter end() { return Iter{ m_storage + m_size }; }
    ConstIter begin() const { return ConstIter{ m_storage }; }
    ConstIter end() const { return ConstIter{ m_storage + m_size }; }
    ConstIter cbegin() const { return ConstIter{ m_storage }; }
    ConstIter cend() const { return ConstIter{ m_storage + m_size }; }
    ReverseIter rbegin() { return ReverseIter{ m_storage + m_size - 1 }; }
    ReverseIter rend() { return ReverseIter{ m_storage - 1 }; }
    ConstReverseIter crbegin() const { return ConstReverseIter{ m_storage + m_size - 1 }; }
    ConstReverseIter crend() const { return ConstReverseIter{ m_storage - 1 }; }
    T& last() { return m_storage[m_size - 1]; }
    const T& last() const { return m_storage[m_size - 1]; }
    void clear_retain() { m_size = 0; }
    // this function completely nukes the vector, if you want to retain capacity, use clear_retain
    void clear() { clear_(); }
    ~Vector() { clear(); }
};
template <typename T>
using RefVector = Vector<RefBox<T>>;
template <Printable T>
struct PrintInfo<Vector<T>> {
    const Vector<T>& m_vec;
    explicit PrintInfo(const Vector<T>& vec) : m_vec(vec) {}
    String repr() {
        if (m_vec.empty()) { return "[]"_s; }
        String con{};
        con.append('[');
        if constexpr (IsSameV<T, String>) {
            for (const auto& s : m_vec) { con.append("\""_s + s + "\", "_s); }
        } else {
            for (const auto& s : m_vec) { con.append(PrintInfo<T>{ s }.repr() + ", "_s); }
        }
        con = con.substring(0, con.size() - 2);
        con.append(']');
        return con;
    }
};
}    // namespace ARLib

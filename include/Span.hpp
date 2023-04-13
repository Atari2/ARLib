#pragma once

#include "Utility.hpp"
#include "Iterator.hpp"
namespace ARLib {
template <typename T>
class Span {
    T* m_begin;
    T* m_end;
    using Iter                          = Iterator<T>;
    using ConstIter                     = ConstIterator<T>;
    constexpr static inline size_t npos = static_cast<size_t>(-1);
    constexpr size_t clamp_count(size_t offset, size_t count) const {
        if (offset + count >= size()) return size() - offset;
        return count;
    }
    constexpr size_t clamp_offset(size_t offset) const {
        if (offset >= size()) return size();
        return offset;
    }

    public:
    constexpr Span() : m_begin(nullptr), m_end(nullptr) {}
    constexpr Span(T* begin, T* end) : m_begin(begin), m_end(end) {}
    constexpr Span(T* begin, size_t size) : m_begin(begin), m_end(begin + size) {}
    template <size_t N>
    constexpr Span(T (&arr)[N]) : m_begin(arr), m_end(arr + N) {}
    constexpr ConstIter begin() const { return ConstIter{ m_begin }; }
    constexpr ConstIter end() const { return ConstIter{ m_end }; }
    constexpr Iter begin() { return Iter{ m_begin }; }
    constexpr Iter end() { return Iter{ m_end }; }
    constexpr size_t size() const { return static_cast<size_t>(m_end - m_begin); }
    constexpr size_t size_bytes() const { return size() * sizeof(T); }
    constexpr T* data() { return m_begin; }
    constexpr const T* data() const { return m_begin; }
    constexpr T& operator[](size_t index) { return m_begin[index]; }
    constexpr const T& operator[](size_t index) const { return m_begin[index]; }
    constexpr T& front() { return *m_begin; }
    constexpr const T& front() const { return *m_begin; }
    constexpr T& back() { return *(m_end - 1); }
    constexpr const T& back() const { return *(m_end - 1); }
    constexpr bool empty() const { return m_begin == m_end; }
    constexpr auto operator[](Pair<size_t, size_t> range) const {
        return subspan(range.first(), range.second() - range.first());
    }
    constexpr auto subspan(size_t offset, size_t count) const {
        offset = clamp_offset(offset);
        count  = clamp_count(offset, count);
        return Span(m_begin + offset, m_begin + offset + count);
    }
    constexpr bool operator==(const Span& other) const noexcept = default;
};
}    // namespace ARLib
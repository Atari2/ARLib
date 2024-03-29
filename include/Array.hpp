#pragma once

#include "Assertion.hpp"
#include "Iterator.hpp"
#include "PrintInfo.hpp"
#include "TypeTraits.hpp"
#include "GenericView.hpp"
#include "Span.hpp"
namespace ARLib {
template <typename T, size_t S>
class Array {
    public:
    T _m_storage_[S];

    private:
    constexpr void assert_size_([[maybe_unused]] size_t index) const {
        SOFT_ASSERT_FMT((index < S), "Index %lu out of bounds in array of size %lu", index, S)
    }

    public:
    constexpr T* data() { return _m_storage_; }
    constexpr const T* data() const { return _m_storage_; }
    constexpr size_t size() const { return S; }
    constexpr T& index(size_t index) {
        assert_size_(index);
        return _m_storage_[index];
    }
    constexpr const T& index(size_t index) const {
        assert_size_(index);
        return _m_storage_[index];
    }
    constexpr T& operator[](size_t index) {
        assert_size_(index);
        return _m_storage_[index];
    }
    constexpr const T& operator[](size_t index) const {
        assert_size_(index);
        return _m_storage_[index];
    }
    constexpr Iterator<T> begin() { return Iterator<T>{ PointerTraits<T*>::pointer_to(*_m_storage_) }; }
    constexpr Iterator<T> end() { return Iterator<T>{ PointerTraits<T*>::pointer_to(*_m_storage_) + S }; }
    constexpr ConstIterator<T> begin() const {
        return ConstIterator<T>{ PointerTraits<const T*>::pointer_to(*_m_storage_) };
    }
    constexpr ConstIterator<T> end() const {
        return ConstIterator<T>{ PointerTraits<const T*>::pointer_to(*_m_storage_) + S };
    }
    constexpr ReverseIterator<T> rbegin() {
        return ReverseIterator<T>{ PointerTraits<T*>::pointer_to(*_m_storage_) + S - 1 };
    }
    constexpr ReverseIterator<T> rend() {
        return ReverseIterator<T>{ PointerTraits<T*>::pointer_to(*_m_storage_) - 1 };
    }
    constexpr ConstReverseIterator<T> rbegin() const {
        return ConstReverseIterator<T>{ ConstReverseIterator<const T*>::pointer_to(*_m_storage_) + S - 1 };
    }
    constexpr ConstIterator<T> rend() const {
        return ConstIterator<T>{ ConstReverseIterator<const T*>::pointer_to(*_m_storage_) - 1 };
    }
    constexpr auto iter() const { return IteratorView{ *this }; }
    constexpr auto iter() { return IteratorView{ *this }; }
    constexpr auto span() const { return Span<const T>{ _m_storage_, S }; }
    constexpr auto span() { return Span<T>{ _m_storage_, S }; }
    constexpr auto enumerate() const {
        ConstEnumerate en{ *this };
        return IteratorView{ en };
    }
    constexpr auto enumerate() {
        Enumerate en{ *this };
        return IteratorView{ en };
    }
};
template <typename First, typename... Rest>
Array(First, Rest...) -> Array<First, 1 + sizeof...(Rest)>;
template <Printable T, size_t S>
struct PrintInfo<Array<T, S>> {
    const Array<T, S>& m_array;
    explicit PrintInfo(const Array<T, S>& array_) : m_array(array_) {}
    String repr() {
        String str{ "[ " };
        for (const auto& v : m_array) { str.append(PrintInfo<T>{ v }.repr() + ", "_s); }
        str = str.substring(0, str.size() - 2);
        str.append(" ]"_s);
        return str;
    }
};
}    // namespace ARLib

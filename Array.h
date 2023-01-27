#pragma once

#include "Assertion.h"
#include "Iterator.h"
#include "PrintInfo.h"
#include "TypeTraits.h"
#include "GenericView.h"
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
    constexpr auto view() const { return IteratorView{ *this }; }
    constexpr auto view() { return IteratorView{ *this }; }
    constexpr auto enumerate() const { ConstEnumerate en{*this}; return IteratorView{ en }; }
    constexpr auto enumerate() { Enumerate en{*this}; return IteratorView{ en }; }
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

#pragma once

#include "Assertion.h"
#include "Iterator.h"
#include "TypeTraits.h"

namespace ARLib {
    template <typename T, size_t S>
    class Array {
        public:
        T _m_storage_[S];

        private:
        void assert_size_(size_t index) {
            SOFT_ASSERT_FMT((index < S), "Index %lu out of bounds in array of size %lu", index, S)
        }

        public:
        constexpr size_t size() const { return S; }

        T& index(size_t index) {
            assert_size_(index);
            return _m_storage_[index];
        }

        const T& index(size_t index) const {
            assert_size_(index);
            return _m_storage_[index];
        }

        T& operator[](size_t index) {
            assert_size_(index);
            return _m_storage_[index];
        }

        const T& operator[](size_t index) const {
            assert_size_(index);
            return _m_storage_[index];
        }

        Iterator<T> begin() { return {PointerTraits<T*>::pointer_to(*_m_storage_)}; }
        Iterator<T> end() { return {PointerTraits<T*>::pointer_to(*_m_storage_) + S}; }

        ConstIterator<T> begin() const { return {PointerTraits<const T*>::pointer_to(*_m_storage_)}; }
        ConstIterator<T> end() const { return {PointerTraits<const T*>::pointer_to(*_m_storage_) + S}; }
    };
} // namespace ARLib

using ARLib::Array;
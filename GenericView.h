#pragma once
#include "Concepts.h"
#include "Iterator.h"

namespace ARLib {
    template <typename T>
    class GenericView {
        T* m_begin_view = nullptr;
        T* m_end_view = nullptr;

        public:
        GenericView(T* begin, T* end) : m_begin_view(begin), m_end_view(m_end_view) {}
        GenericView(T* begin, size_t size) : m_begin_view(begin), m_end_view(begin + size) {}

        // this constructor works only if the container operates on contiguous memory (e.g. not a linked list)
        // actually this whole class only operates on contiguous memory
        GenericView(const Iterable auto& container) {
            m_begin_view = &(*container.begin());
            m_end_view = &(*container.end()) - 1;
        }

        size_t size() { return m_end_view - m_begin_view; }

        Iterator<T> begin() { return {m_begin_view}; }

        Iterator<T> end() { return {m_end_view + 1}; }

        ReverseIterator<T> rbegin() { return {m_end_view}; }

        ReverseIterator<T> rend() { return {m_begin_view - 1}; }

        ConstIterator<T> begin() const { return {m_begin_view}; }

        ConstIterator<T> end() const { return {m_end_view + 1}; }

        ConstReverseIterator<T> rbegin() const { return {m_end_view}; }

        ConstReverseIterator<T> rend() const { return {m_begin_view - 1}; }

        T& operator[](size_t index) { return m_begin_view[index]; }

        const T& operator[](size_t index) const { return m_begin_view[index]; }

        const T* data() const { return m_begin_view; }

        T* data() { return m_begin_view; }

        template <typename Functor>
        void for_each(Functor func) {
            for (auto& item : *this) {
                func(item);
            }
        }
    };
} // namespace ARLib

using ARLib::GenericView;
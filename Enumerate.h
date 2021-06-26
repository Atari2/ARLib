#pragma once
#include "Algorithm.h"
#include "Iterator.h"

namespace ARLib {
    template <EnumerableC T>
    class Enumerate {
        using TRef = typename RemoveReference<T>::type&;
        TRef m_container;

        public:
        Enumerate(T& container) : m_container(container) {}

        auto begin() const { return Enumerator{ARLib::begin(m_container), 0ull}; }
        auto end() const { return Enumerator{ARLib::end(m_container), m_container.size()}; }
    };
} // namespace ARLib

using ARLib::Enumerate;
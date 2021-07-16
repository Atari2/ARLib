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

    // simple iterator pair wrapper, will only iterate until the shorter of the 2 iterators has elements.
    template <Iterable F, Iterable S>
    class PairIterate {
        F& m_first;
        S& m_second;

        public:
        PairIterate(F& f, S& s) : m_first(f), m_second(s) {}

        auto begin() const { return PairIterator{m_first.begin(), m_second.begin()}; }
        auto end() const { return PairIterator{m_first.end(), m_second.end()}; }
    };
} // namespace ARLib

using ARLib::Enumerate;
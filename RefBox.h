#pragma once
#include "TypeTraits.h"

namespace ARLib {

    // simple box for a reference type, should not be used
    template <typename T>
    class RefBox {

        using TRef = typename RemoveReference<T>::type&;
        TRef m_boxed;

        public:
        RefBox(TRef obj) : m_boxed(obj) {}
        RefBox(const RefBox& other) : m_boxed(other.m_boxed) { }
        RefBox& operator=(const RefBox& other) {
            m_boxed = other.m_boxed;
            return *this;
        }

        TRef operator*() { return m_boxed; }
        const TRef operator*() const { return m_boxed; }
        auto operator->() { return &m_boxed; }
        auto operator->() const { return &m_boxed; }
        TRef get() { return m_boxed; }
    };
} // namespace ARLib

using ARLib::RefBox;

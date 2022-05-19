#pragma once
#include "TypeTraits.h"

namespace ARLib {

    // simple box for a reference type, should not be used
    template <typename T>
    class RefBox {
        using TRef = AddLvalueReferenceT<RemoveReferenceT<T>>;
        using CTRef = AddConstT<AddLvalueReferenceT<RemoveReferenceT<T>>>;
        using TPtr = AddPointerT<RemoveReferenceT<T>>;
        TPtr m_boxed{};

        public:
        explicit RefBox(TRef obj) : m_boxed(&obj) {}

        TRef operator*() { return *m_boxed; }
        TRef operator*() const { return *m_boxed; }
        auto operator->() { return m_boxed; }
        auto operator->() const { return m_boxed; }
        CTRef get() const { return *m_boxed; }
        TRef get() { return *m_boxed; }
    };
} // namespace ARLib

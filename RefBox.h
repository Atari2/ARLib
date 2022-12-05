#pragma once
#include "TypeTraits.h"
namespace ARLib {
// simple box for a reference type, should not be used
template <typename T>
class RefBox {
    using TRef  = AddLvalueReferenceT<RemoveReferenceT<T>>;
    using CTRef = AddConstT<AddLvalueReferenceT<RemoveReferenceT<T>>>;
    using TPtr  = AddPointerT<RemoveReferenceT<T>>;
    TPtr m_boxed{};

    public:
    constexpr RefBox() : m_boxed(nullptr) {}
    constexpr explicit RefBox(TRef obj) : m_boxed(&obj) {}
    constexpr TRef operator*() { return *m_boxed; }
    constexpr TRef operator*() const { return *m_boxed; }
    constexpr auto operator->() { return m_boxed; }
    constexpr auto operator->() const { return m_boxed; }
    constexpr CTRef get() const { return *m_boxed; }
    constexpr TRef get() { return *m_boxed; }
};
}    // namespace ARLib

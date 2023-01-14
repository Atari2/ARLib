#pragma once
#include "TypeTraits.h"
#include "PrintInfo.h"
namespace ARLib {
// simple box for a reference type, should not be used
template <typename T>
class RefBox {
    using TRef  = AddLvalueReferenceT<RemoveReferenceT<T>>;
    using CTRef = AddConstT<AddLvalueReferenceT<RemoveReferenceT<T>>>;
    using TPtr  = AddPointerT<RemoveReferenceT<T>>;
    using CTPtr = AddConstT<AddPointerT<RemoveReferenceT<T>>>;
    TPtr m_boxed{};

    public:
    constexpr RefBox() : m_boxed(nullptr) {}
    constexpr RefBox(TRef obj) : m_boxed(&obj) {}
    constexpr CTPtr ptr() const { return m_boxed; }
    constexpr TPtr ptr() { return m_boxed; }
    constexpr operator TRef() { return *m_boxed; }
    constexpr operator CTRef() const { return *m_boxed; }
    constexpr TRef operator*() { return *m_boxed; }
    constexpr CTRef operator*() const { return *m_boxed; }
    constexpr auto operator->() { return m_boxed; }
    constexpr auto operator->() const { return m_boxed; }
    constexpr CTRef get() const { return *m_boxed; }
    constexpr TRef get() { return *m_boxed; }
};

template <typename T>
struct PrintInfo<RefBox<T>> {
    const RefBox<T>& m_loc;
    explicit PrintInfo(const RefBox<T>& loc) : m_loc(loc) {}
    String repr() {
        if (m_loc.ptr() == nullptr) return "RefBox { nullptr }"_s;
        return "RefBox { "_s + print_conditional(m_loc.get()) + " }"_s;
    }
};

}    // namespace ARLib

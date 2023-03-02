#pragma once
#include "TypeTraits.h"
#include "PrintInfo.h"
namespace ARLib {
// simple box for a reference type, should not be used
template <typename T>
class RefBox {
    using TPtr = AddPointerT<RemoveReferenceT<T>>;
    TPtr m_boxed{};

    public:
    constexpr RefBox() : m_boxed(nullptr) {}
    constexpr RefBox(T& obj) : m_boxed(&obj) {}
    constexpr const TPtr ptr() const { return m_boxed; }
    constexpr TPtr ptr() { return m_boxed; }
    constexpr operator T&() { return *m_boxed; }
    constexpr T& operator*() { return *m_boxed; }
    constexpr const T& operator*() const { return *m_boxed; }
    constexpr auto operator->() { return m_boxed; }
    constexpr auto operator->() const { return m_boxed; }
    constexpr const auto& get() const { return *m_boxed; }
    constexpr auto& get() { return *m_boxed; }
    constexpr bool operator==(const RefBox& other) const {
        if (m_boxed && other.m_boxed) { return *m_boxed == *other.m_boxed; }
        return m_boxed == other.m_boxed;
    }
    constexpr bool operator!=(const RefBox& other) const {
        if (m_boxed && other.m_boxed) { return *m_boxed != *other.m_boxed; }
        return m_boxed != other.m_boxed;
    }
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

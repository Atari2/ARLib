#pragma once
#include "PrintInfo.hpp"
#include "Types.hpp"
#include "Utility.hpp"

#ifdef COMPILER_MSVC
    #include <intrin.h>
    #define SYNC_INC(x) _InterlockedIncrement(x)
    #define SYNC_DEC(x) _InterlockedDecrement(x)
#else
    #define SYNC_INC(x) __sync_add_and_fetch(x, 1)
    #define SYNC_DEC(x) __sync_sub_and_fetch(x, 1)
#endif
namespace ARLib {
template <typename T, bool Multiple = false>
class RefCountBase {
    unsigned long m_counter   = 1;
    unsigned long m_weak_refs = 0;
    T* m_object               = nullptr;
    void destroy() noexcept {
        if constexpr (Multiple) {
            delete[] m_object;
        } else {
            delete m_object;
        }
    }

    public:
    constexpr RefCountBase() noexcept = default;
    explicit RefCountBase(T* object) : m_object(object) {}
    RefCountBase(const RefCountBase&)            = delete;
    RefCountBase& operator=(const RefCountBase&) = delete;
    void incref() noexcept { SYNC_INC(cast<volatile long*>(&m_counter)); }
    void incweakref() noexcept { SYNC_INC(cast<volatile long*>(&m_weak_refs)); }
    void decref() noexcept {
        if (SYNC_DEC(cast<volatile long*>(&m_counter)) == 0) { destroy(); }
    }
    void decweakref() noexcept { SYNC_DEC(cast<volatile long*>(&m_weak_refs)); }
    T* release_storage() {
        T* ptr   = m_object;
        m_object = nullptr;
        return ptr;
    }
    auto count() const noexcept { return m_counter; }
    auto weak_count() const noexcept { return m_weak_refs; }
};

template <typename T>
class SharedPtr;
template <typename T>
class WeakPtr {
    T* m_storage             = nullptr;
    RefCountBase<T>* m_count = nullptr;
    friend SharedPtr<T>;
    WeakPtr(T* storage_ptr, RefCountBase<T>* count) : m_storage(storage_ptr), m_count(count) { m_count->incweakref(); }
    void decrease_instance_count_() {
        if (m_count == nullptr) return;
        m_count->decweakref();
    }
    public:
    T* get() { return m_storage; }
    const T* get() const { return m_storage; }
    auto refcount() const { return m_count ? m_count->count() : 0ul; }
    SharedPtr<T> lock();
    bool exists() const { return m_storage != nullptr; }
    T* operator->() { return m_storage; }
    const T* operator->() const { return m_storage; }
    T& operator*() { return *m_storage; }
    const T& operator*() const { return *m_storage; }
    ~WeakPtr() { decrease_instance_count_(); }
};
template <Printable T>
struct PrintInfo<WeakPtr<T>> {
    const WeakPtr<T>& m_ptr;
    PrintInfo(const WeakPtr<T>& ptr) : m_ptr(ptr) {}
    String repr() const { return "WeakPtr { "_s + PrintInfo<T>{ *m_ptr.get() }.repr() + " }"_s; }
};
}    // namespace ARLib

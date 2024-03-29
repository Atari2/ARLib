#pragma once
#include "Conversion.hpp"
#include "HashBase.hpp"
#include "Memory.hpp"
#include "PrintInfo.hpp"
#include "TypeTraits.hpp"
#include "WeakPtr.hpp"
namespace ARLib {
template <typename T>
class SharedPtr {
    T* m_storage             = nullptr;
    RefCountBase<T>* m_count = nullptr;
    void decrease_instance_count_() {
        if (m_count == nullptr) return;
        m_count->decref();
        if (m_count->count() == 0) {
            delete m_count;
            m_count = nullptr;
        }
    }

    SharedPtr(WeakPtr<T>& weak) { 
        m_storage = weak.m_storage;
        m_count   = weak.m_count;
        m_count->incref();
    }

    public:
    constexpr SharedPtr() = default;
    SharedPtr(SharedPtr&& other) noexcept : m_storage(other.m_storage), m_count(other.m_count) {
        other.m_storage = nullptr;
        other.m_count   = nullptr;
    }
    SharedPtr& operator=(SharedPtr&& other) noexcept {
        decrease_instance_count_();
        m_storage       = other.m_storage;
        m_count         = other.m_count;
        other.m_storage = nullptr;
        other.m_count   = nullptr;
        return *this;
    }
    SharedPtr(nullptr_t) = delete;
    SharedPtr(T* ptr) : m_storage(ptr), m_count(new RefCountBase<T>{ m_storage }) {
        HARD_ASSERT(ptr, "Pointer passed to SharedPtr must not be null");
    }
    SharedPtr(T&& storage) {
        m_storage = new T{ move(storage) };
        m_count   = new RefCountBase<T>{ m_storage };
    }
    SharedPtr(const SharedPtr& other) {
        m_storage = other.m_storage;
        m_count   = other.m_count;
        if (m_storage == nullptr && m_count == nullptr) return;
        m_count->incref();
    }
    template <typename... Args>
    SharedPtr(EmplaceT<T>, Args&&... args) {
        m_storage = new T{ Forward<Args>(args)... };
        m_count   = new RefCountBase<T>{ m_storage };
    }
    SharedPtr& operator=(const SharedPtr& other) {
        if (this == &other) return *this;
        reset();
        m_storage = other.m_storage;
        m_count   = other.m_count;
        if (m_storage == nullptr || m_count == nullptr) return *this;
        m_count->incref();
        return *this;
    }
    bool operator==(const SharedPtr& other) const { return m_storage == other.m_storage; }
    bool operator==(const T* other_ptr) const { return m_storage == other_ptr; }
    T* release() {
        T* ptr = m_count->release_storage();
        decrease_instance_count_();
        m_count   = nullptr;
        m_storage = nullptr;
        return ptr;
    }
    void reset() {
        decrease_instance_count_();
        m_storage = nullptr;
        m_count   = nullptr;
    }
    void share_with(SharedPtr& other) const {
        other.m_storage = m_storage;
        other.m_count   = m_count;
        m_count->incref();
    }
    WeakPtr<T> weakptr() const { return WeakPtr{ m_storage, m_count }; }
    T* get() { return m_storage; }
    const T* get() const { return m_storage; }
    auto refcount() const { return m_count ? m_count->count() : 0ul; }
    bool exists() const { return m_storage != nullptr; }
    T* operator->() { return m_storage; }
    const T* operator->() const { return m_storage; }
    T& operator*() { return *m_storage; }
    const T& operator*() const { return *m_storage; }
    ~SharedPtr() { decrease_instance_count_(); }
};

template <typename T>
SharedPtr<T> WeakPtr<T>::lock() {
    return SharedPtr{ *this };
}

template <typename T>
class SharedPtr<T[]> {
    using RefCount    = RefCountBase<T, true>;
    T* m_storage      = nullptr;
    RefCount* m_count = nullptr;
    size_t m_size     = 0ull;
    void decrease_instance_count_() {
        if (m_count == nullptr) return;
        m_count->decref();
        if (m_count->count() == 0) {
            delete m_count;
            m_count = nullptr;
        }
    }

    public:
    constexpr SharedPtr() = default;
    SharedPtr(SharedPtr&& other) noexcept : m_storage(other.m_storage), m_count(other.m_count) {
        other.m_storage = nullptr;
        other.m_count   = nullptr;
    }
    SharedPtr& operator=(SharedPtr&& other) noexcept {
        decrease_instance_count_();
        m_storage       = other.m_storage;
        m_count         = other.m_count;
        other.m_storage = nullptr;
        other.m_count   = nullptr;
        return *this;
    }
    SharedPtr(T* ptr, size_t size) : m_storage(new T[size]), m_count(new RefCount{ m_storage }), m_size(size) {
        ConditionalBitCopy(m_storage, ptr, size);
    }
    template <size_t N>
    SharedPtr(T (&src)[N]) : m_storage(new T[N]), m_count(new RefCount{ m_storage }), m_size(N) {
        ConditionalBitCopy(m_storage, src, N);
    }
    SharedPtr(const SharedPtr& other) {
        m_storage = other.m_storage;
        m_count   = other.m_count;
        m_count->incref();
    }
    SharedPtr& operator=(const SharedPtr& other) {
        if (this == &other) return *this;
        reset();
        m_storage = other.m_storage;
        m_count   = other.m_count;
        m_count->incref();
        return *this;
    }
    bool operator==(const SharedPtr& other) const { return m_storage == other.m_storage; }
    T* release() {
        T* ptr = m_count->release_storage();
        decrease_instance_count_();
        m_count   = nullptr;
        m_storage = nullptr;
        return ptr;
    }
    void reset() {
        decrease_instance_count_();
        m_storage = nullptr;
    }
    void share_with(SharedPtr& other) const {
        other.m_storage = m_storage;
        other.m_count   = m_count;
        m_count->incref();
    }
    size_t size() const { return m_size; }
    T* get() { return m_storage; }
    const T* get() const { return m_storage; }
    auto refcount() const { return m_count ? m_count->count() : 0ul; }
    bool exists() const { return m_storage != nullptr; }
    T* operator->() { return m_storage; }
    T& operator[](size_t index) { return m_storage[index]; }
    const T* operator->() const { return m_storage; }
    const T& operator[](size_t index) const { return m_storage[index]; }
    ~SharedPtr() { decrease_instance_count_(); }
};
template <class T>
struct Hash<SharedPtr<T>> {
    [[nodiscard]] size_t operator()(const SharedPtr<T>& ptr) const noexcept {
        return reinterpret_cast<uintptr_t>(ptr.get());
    }
};
template <typename T>
struct PrintInfo<SharedPtr<T>> {
    const SharedPtr<T>& m_ptr;
    PrintInfo(const SharedPtr<T>& ptr) : m_ptr(ptr) {}
    String repr() const {
        if constexpr (Printable<T>) {
            return "SharedPtr { "_s + PrintInfo<T>{ *m_ptr.get() }.repr() + " }"_s;
        } else {
            DemangledInfo info{ MANGLED_TYPENAME_TO_STRING(T) };
            return "SharedPtr { "_s + String{ info.name() } + " }"_s;
        };
    }
};
template <Printable T>
struct PrintInfo<SharedPtr<T[]>> {
    const SharedPtr<T[]>& m_ptr;
    PrintInfo(const SharedPtr<T[]>& ptr) : m_ptr(ptr) {}
    String repr() const {
        String conc{};
        if (m_ptr.exists()) {
            if (m_ptr.size() > 0) conc.append("[ ");
            for (size_t i = 0; i < m_ptr.size(); i++) {
                conc.append(PrintInfo<T>{ m_ptr[i] }.repr());
                conc.append(", ");
            }
            conc = conc.substring(0, conc.size() - 2);
            conc.append(" ]");
        } else {
            conc.append("nullptr");
        }
        return "SharedPtr { "_s + conc + " }"_s;
    }
};
}    // namespace ARLib

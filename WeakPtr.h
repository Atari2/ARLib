#pragma once
#include "PrintInfo.h"
#include "Types.h"
#include "Utility.h"

namespace ARLib {
    template <typename T>
    class WeakPtr {
        T* m_storage = nullptr;

        public:
        constexpr WeakPtr() = default;
        WeakPtr(const WeakPtr&) = default;
        WeakPtr(WeakPtr&&) = default;
        WeakPtr& operator=(const WeakPtr&) = default;
        WeakPtr& operator=(WeakPtr&&) = default;

        explicit WeakPtr(nullptr_t) = delete;
        explicit WeakPtr(T* ptr) : m_storage(ptr) {}
        explicit WeakPtr(T& ref) : m_storage(addressof(ref)) {}

        void exchange(T*& ptr) { swap(ptr, m_storage); }

        void adopt(T* ptr) { m_storage = ptr; }

        T* release() {
            T* ptr = m_storage;
            m_storage = nullptr;
            return ptr;
        }

        bool operator==(const WeakPtr& other) const { return m_storage == other.m_storage; }
        bool operator!=(const WeakPtr& other) const { return m_storage != other.m_storage; }

        T* get() { return m_storage; }
        const T* get() const { return m_storage; }

        bool exists() const { return m_storage != nullptr; }
        explicit operator bool() const { return m_storage != nullptr; }

        T* operator->() { return m_storage; }
        const T* operator->() const { return m_storage; }

        T& operator*() { return *m_storage; }
        const T& operator*() const { return *m_storage; }
    };

    template <Printable T>
    struct PrintInfo<WeakPtr<T>> {
        const WeakPtr<T>& m_ptr;
        PrintInfo(const WeakPtr<T>& ptr) : m_ptr(ptr) {}
        String repr() const { return "WeakPtr { "_s + PrintInfo<T>{*m_ptr.get()}.repr() + " }"_s; }
    };
} // namespace ARLib
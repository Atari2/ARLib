#pragma once
#include "HashBase.h"
#include "Utility.h"

namespace ARLib {
    template <class T>
    class UniquePtr {
        T* m_storage = nullptr;

        public:
        constexpr UniquePtr() = default;
        UniquePtr(const UniquePtr&) = delete;
        UniquePtr& operator=(const UniquePtr&) = delete;

        UniquePtr(T* ptr) : m_storage(ptr) {}
        UniquePtr(T&& storage) { m_storage = new T{move(storage)}; }
        UniquePtr(UniquePtr&& ptr) {
            m_storage = ptr.m_storage;
            ptr.m_storage = nullptr;
        }
        template <typename... Args>
        UniquePtr(EmplaceT<T>, Args&&... args) {
            m_storage = new T{Forward<Args>(args)...};
        }

        UniquePtr& operator=(UniquePtr&& other) {
            m_storage = other.m_storage;
            other.m_storage = nullptr;
            return *this;
        }

        T* release() {
            T* ptr = m_storage;
            m_storage = nullptr;
            return ptr;
        }

        void reset() {
            delete m_storage;
            m_storage = nullptr;
        }

        T* get() { return m_storage; }

        bool exists() { return m_storage != nullptr; }

        T* operator->() { return m_storage; }

        T& operator*() { return *m_storage; }

        ~UniquePtr() { delete m_storage; }
    };

    template <class T>
    class UniquePtr<T[]> {
        T* m_storage;

        public:
        constexpr UniquePtr() = default;
        UniquePtr(const UniquePtr&) = delete;
        UniquePtr& operator=(const UniquePtr&) = delete;

        UniquePtr(size_t size) { m_storage = new T[size]; }
        UniquePtr(T* ptr) : m_storage(ptr) {}
        UniquePtr(UniquePtr&& ptr) {
            m_storage = ptr.m_storage;
            ptr.m_storage = nullptr;
        }
        UniquePtr& operator=(UniquePtr&& other) {
            m_storage = other.m_storage;
            other.m_storage = nullptr;
            return *this;
        }

        T* release() {
            T* ptr = m_storage;
            m_storage = nullptr;
            return ptr;
        }

        void reset() {
            delete[] m_storage;
            m_storage = nullptr;
        }

        T* get() { return m_storage; }

        bool exists() { return m_storage != nullptr; }

        T& operator[](size_t index) { return m_storage[index]; }

        ~UniquePtr() { delete[] m_storage; }
    };

    template <class T>
    class Hash<UniquePtr<T>> {
        [[nodiscard]] size_t operator()(const UniquePtr<T>& key) const noexcept { return Hash<T>{}(); }
    };

} // namespace ARLib

using ARLib::UniquePtr;
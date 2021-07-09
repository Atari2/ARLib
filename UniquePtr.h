#pragma once
#include "Algorithm.h"
#include "HashBase.h"
#include "Memory.h"
#include "Utility.h"

namespace ARLib {
    template <class T>
    class UniquePtr {
        T* m_storage = nullptr;

        public:
        constexpr UniquePtr() = default;
        UniquePtr(const UniquePtr& other) {
            if (other.m_storage) { m_storage = new T(*other.m_storage); }
        }
        UniquePtr& operator=(const UniquePtr& other) {
            delete m_storage;
            if (other.m_storage) { m_storage = new T(*other.m_storage); }
            return *this;
        }

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
        T* m_storage = nullptr;
        size_t m_size = 0ull;

        public:
        constexpr UniquePtr() = default;
        UniquePtr(const UniquePtr& other) : m_storage(new T[other.m_size]), m_size(other.m_size) {
            if (other.m_storage) { ConditionalBitCopy(m_storage, other.m_storage, m_size); }
        }
        UniquePtr& operator=(const UniquePtr& other) {
            delete[] m_storage;
            m_size = other.m_size;
            if (other.m_storage) {
                m_storage = new T[m_size];
                ConditionalBitCopy(m_storage, other.m_storage, m_size);
            }
            return *this;
        }

        UniquePtr(size_t size) : m_storage(new T[size]), m_size(size) {}

        template <size_t N>
        UniquePtr(T (&src)[N]) : m_storage(new T[N]), m_size(N) {
            ConditionalBitCopy(m_storage, src, N);
        }

        UniquePtr(T* ptr, size_t size) : m_storage(new T[size]), m_size(size) {
            ConditionalBitCopy(m_storage, ptr, m_size);
        }
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

        size_t size() { return m_size; }

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
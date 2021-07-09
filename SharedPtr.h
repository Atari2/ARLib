#pragma once
#include "TypeTraits.h"
#include "Memory.h"

namespace ARLib {
    template <typename T>
    class SharedPtr {
        T* m_storage = nullptr;
        size_t* m_count = nullptr;

        bool decrease_instance_count_() {
            if (m_count == nullptr) return false;
            (*m_count)--;
            if (*m_count == 0) {
                delete m_count;
                m_count = nullptr;
                return true;
            }
            return false;
        }

        public:
        constexpr SharedPtr() = default;
        SharedPtr(SharedPtr&& other) : m_storage(other.m_storage), m_count(other.m_count) {
            other.m_storage = nullptr;
            other.m_count = nullptr;
        }
        SharedPtr& operator=(SharedPtr&& other) {
            if (decrease_instance_count_()) { delete m_storage; }
            m_storage = other.m_storage;
            m_count = other.m_count;
            other.m_storage = nullptr;
            other.m_count = nullptr;
            return *this;
        }

        SharedPtr(T* ptr) : m_storage(ptr), m_count(new size_t{1}) {}
        SharedPtr(T&& storage) {
            m_storage = new T{move(storage)};
            m_count = new size_t{1};
        }
        SharedPtr(const SharedPtr& other) {
            m_storage = other.m_storage;
            m_count = other.m_count;
            (*m_count)++;
        }

        template <typename... Args>
        SharedPtr(EmplaceT<T>, Args&&... args) {
            m_storage = new T{Forward<Args>(args)...};
            m_count = new size_t{1};
        }

        SharedPtr& operator=(const SharedPtr& other) {
            reset();
            m_storage = other.m_storage;
            m_count = other.m_count;
            (*m_count)++;
            return *this;
        }
        T* release() {
            T* ptr = m_storage;
            decrease_instance_count_();
            m_count = nullptr;
            m_storage = nullptr;
            return ptr;
        }

        void reset() {
            if (decrease_instance_count_()) { delete m_storage; }
            m_storage = nullptr;
        }

        void share_with(SharedPtr& other) {
            other.m_storage = m_storage;
            other.m_count = m_count;
            (*m_count)++;
        }

        T* get() { return m_storage; }

        size_t refcount() { return m_count ? *m_count : 0; }
        bool exists() { return m_storage != nullptr; }

        T* operator->() { return m_storage; }

        T& operator*() { return *m_storage; }

        ~SharedPtr() {
            if (decrease_instance_count_()) { delete m_storage; }
        }
    };

    template <typename T>
    class SharedPtr<T[]> {
        T* m_storage = nullptr;
        size_t* m_count = nullptr;
        size_t m_size = 0ull;

        bool decrease_instance_count_() {
            if (m_count == nullptr) return false;
            (*m_count)--;
            if (*m_count == 0) {
                delete m_count;
                m_count = nullptr;
                return true;
            }
            return false;
        }

        public:
        constexpr SharedPtr() = default;
        SharedPtr(SharedPtr&& other) : m_storage(other.m_storage), m_count(other.m_count) {
            other.m_storage = nullptr;
            other.m_count = nullptr;
        }
        SharedPtr& operator=(SharedPtr&& other) {
            if (decrease_instance_count_()) { delete m_storage; }
            m_storage = other.m_storage;
            m_count = other.m_count;
            other.m_storage = nullptr;
            other.m_count = nullptr;
            return *this;
        }

        SharedPtr(T* ptr, size_t size) : m_storage(new T[size]), m_count(new size_t{1}), m_size(size) {
            ConditionalBitCopy(m_storage, ptr, size);
        }

        template <size_t N>
        SharedPtr(T (&src)[N]) : m_storage(new T[N]), m_count(new size_t{1}), m_size(N) {
            ConditionalBitCopy(m_storage, src, N);
        }
        SharedPtr(const SharedPtr& other) {
            m_storage = other.m_storage;
            m_count = other.m_count;
            (*m_count)++;
        }

        SharedPtr& operator=(const SharedPtr& other) {
            reset();
            m_storage = other.m_storage;
            m_count = other.m_count;
            (*m_count)++;
            return *this;
        }
        T* release() {
            T* ptr = m_storage;
            decrease_instance_count_();
            m_count = nullptr;
            m_storage = nullptr;
            return ptr;
        }

        void reset() {
            if (decrease_instance_count_()) { delete[] m_storage; }
            m_storage = nullptr;
        }

        void share_with(SharedPtr& other) {
            other.m_storage = m_storage;
            other.m_count = m_count;
            (*m_count)++;
        }

        size_t size() { return m_size; }

        T* get() { return m_storage; }

        size_t refcount() { return m_count ? *m_count : 0; }
        bool exists() { return m_storage != nullptr; }

        T* operator->() { return m_storage; }

        T& operator[](size_t index) { return m_storage[index]; }

        ~SharedPtr() {
            if (decrease_instance_count_()) { delete[] m_storage; }
        }
    };
} // namespace ARLib

using ARLib::SharedPtr;
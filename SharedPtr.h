#pragma once
#include "HashBase.h"
#include "Memory.h"
#include "PrintInfo.h"
#include "TypeTraits.h"

namespace ARLib {
    template <typename T>
    class SharedPtr {
        T* m_storage = nullptr;
        size_t* m_count = nullptr;

        bool decrease_instance_count_() {
            if (m_count == nullptr) return false;
            if (*m_count == 0) {
                // count may be 0 if object was constructed from nullptr
                // which is valid, for now, since it makes life a little bit easier
                delete m_count;
                m_count = nullptr;
                return true;
            }
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
        SharedPtr(SharedPtr&& other) noexcept : m_storage(other.m_storage), m_count(other.m_count) {
            other.m_storage = nullptr;
            other.m_count = nullptr;
        }
        SharedPtr& operator=(SharedPtr&& other) noexcept {
            if (decrease_instance_count_()) { delete m_storage; }
            m_storage = other.m_storage;
            m_count = other.m_count;
            other.m_storage = nullptr;
            other.m_count = nullptr;
            return *this;
        }
        SharedPtr(nullptr_t) : m_storage(nullptr), m_count(new size_t{0}) {}
        SharedPtr(T* ptr) : m_storage(ptr), m_count(ptr ? new size_t{1} : new size_t{0}) {}
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
            if (this == &other) return *this;
            reset();
            m_storage = other.m_storage;
            m_count = other.m_count;
            (*m_count)++;
            return *this;
        }

        bool operator==(const SharedPtr& other) const { return m_storage == other.m_storage; }
        bool operator==(const T* other_ptr) const { return m_storage == other_ptr; }

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
            m_count = nullptr;
        }

        void share_with(SharedPtr& other) const {
            other.m_storage = m_storage;
            other.m_count = m_count;
            (*m_count)++;
        }

        T* get() { return m_storage; }
        const T* get() const { return m_storage; }

        size_t refcount() const { return m_count ? *m_count : 0; }
        bool exists() const { return m_storage != nullptr; }

        T* operator->() { return m_storage; }
        const T* operator->() const { return m_storage; }
        T& operator*() { return *m_storage; }
        const T& operator*() const { return *m_storage; }

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
        SharedPtr(SharedPtr&& other) noexcept : m_storage(other.m_storage), m_count(other.m_count) {
            other.m_storage = nullptr;
            other.m_count = nullptr;
        }
        SharedPtr& operator=(SharedPtr&& other) noexcept {
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
            if (this == &other) return *this;
            reset();
            m_storage = other.m_storage;
            m_count = other.m_count;
            (*m_count)++;
            return *this;
        }

        bool operator==(const SharedPtr& other) const { return m_storage == other.m_storage; }

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

        void share_with(SharedPtr& other) const {
            other.m_storage = m_storage;
            other.m_count = m_count;
            (*m_count)++;
        }

        size_t size() const { return m_size; }

        T* get() { return m_storage; }
        const T* get() const { return m_storage; }

        size_t refcount() const { return m_count ? *m_count : 0; }
        bool exists() const { return m_storage != nullptr; }

        T* operator->() { return m_storage; }

        T& operator[](size_t index) { return m_storage[index]; }

        const T* operator->() const { return m_storage; }

        const T& operator[](size_t index) const { return m_storage[index]; }

        ~SharedPtr() {
            if (decrease_instance_count_()) { delete[] m_storage; }
        }
    };

    template <class T>
    struct Hash<SharedPtr<T>> {
        [[nodiscard]] size_t operator()(const SharedPtr<T>& ptr) const noexcept {
            return reinterpret_cast<uintptr_t>(ptr.get());
        }
    };

    template <Printable T>
    struct PrintInfo<SharedPtr<T>> {
        const SharedPtr<T>& m_ptr;
        PrintInfo(const SharedPtr<T>& ptr) : m_ptr(ptr) {}
        String repr() const { return "SharedPtr { "_s + PrintInfo<T>{*m_ptr.get()}.repr() + " }"_s; }
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
                    conc.append(PrintInfo<T>{m_ptr[i]}.repr());
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
} // namespace ARLib

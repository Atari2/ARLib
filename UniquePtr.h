#pragma once
#include "HashBase.h"
#include "Memory.h"
#include "PrintInfo.h"
#include "Utility.h"

namespace ARLib {
    template <class T>
    class UniquePtr {
        T* m_storage = nullptr;

        public:
        constexpr UniquePtr() = default;
        constexpr UniquePtr(nullptr_t) : m_storage(nullptr){};
        UniquePtr(const UniquePtr&) = delete;

        explicit UniquePtr(T* ptr) : m_storage(ptr) {}
        explicit UniquePtr(T&& storage) : m_storage(new T{move(storage)}) {}
        UniquePtr(UniquePtr&& ptr) noexcept {
            m_storage = ptr.m_storage;
            ptr.m_storage = nullptr;
        }
        template <typename... Args>
        explicit UniquePtr(EmplaceT<T>, Args&&... args) {
            m_storage = new T{Forward<Args>(args)...};
        }

        UniquePtr& operator=(UniquePtr&& other) noexcept {
            m_storage = other.m_storage;
            other.m_storage = nullptr;
            return *this;
        }
        UniquePtr& operator=(const UniquePtr&) = delete;

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
        const T* get() const { return m_storage; }

        bool exists() const { return m_storage != nullptr; }
        explicit operator bool() const { return m_storage != nullptr; }

        T* operator->() { return m_storage; }
        const T* operator->() const { return m_storage; }

        T& operator*() { return *m_storage; }
        const T& operator*() const { return *m_storage; }

        ~UniquePtr() { delete m_storage; }
    };

    template <class T>
    class UniquePtr<T[]> {
        T* m_storage = nullptr;
        size_t m_size = 0ull;

        public:
        constexpr UniquePtr() = default;
        constexpr UniquePtr(nullptr_t) : m_storage(nullptr){};
        UniquePtr(const UniquePtr&) = delete;
        UniquePtr& operator=(const UniquePtr&) = delete;

        explicit UniquePtr(size_t size) requires DefaultConstructible<T> : m_storage(new T[size]), m_size(size) {
            for (size_t i = 0; i < size; i++)
                m_storage[i] = T{};
        }

        template <size_t N>
        explicit UniquePtr(T (&src)[N]) : m_storage(new T[N]), m_size(N) {
            ConditionalBitCopy(m_storage, src, N);
        }

        UniquePtr(T* ptr, size_t size) : m_storage(new T[size]), m_size(size) {
            ConditionalBitCopy(m_storage, ptr, m_size);
        }
        UniquePtr(UniquePtr&& ptr) noexcept {
            m_storage = ptr.m_storage;
            ptr.m_storage = nullptr;
        }
        UniquePtr& operator=(UniquePtr&& other) noexcept {
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

        size_t size() const { return m_size; }

        T* get() { return m_storage; }
        const T* get() const { return m_storage; }

        bool exists() const { return m_storage != nullptr; }

        T& operator[](size_t index) { return m_storage[index]; }
        const T& operator[](size_t index) const { return m_storage[index]; }

        ~UniquePtr() { delete[] m_storage; }
    };

    template <class T>
    struct Hash<UniquePtr<T>> {
        [[nodiscard]] size_t operator()(const UniquePtr<T>& ptr) const noexcept {
            return reinterpret_cast<uintptr_t>(ptr.get());
        }
    };

    template <typename T>
    struct PrintInfo<UniquePtr<T>> {
        const UniquePtr<T>& m_ptr;
        PrintInfo(const UniquePtr<T>& ptr) : m_ptr(ptr) {}
        String repr() const {
            if constexpr (Printable<T>) {
                return "UniquePtr { "_s + PrintInfo<T>{*m_ptr.get()}.repr() + " }"_s;
            } else {
                DemangledInfo info{MANGLED_TYPENAME_TO_STRING(T)};
                return "UniquePtr { "_s + String{info.name()} + " }"_s;
            }
        }
    };

    template <Printable T>
    struct PrintInfo<UniquePtr<T[]>> {
        const UniquePtr<T[]>& m_ptr;
        PrintInfo(const UniquePtr<T[]>& ptr) : m_ptr(ptr) {}
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
            return "UniquePtr { "_s + conc + " }"_s;
        }
    };
} // namespace ARLib

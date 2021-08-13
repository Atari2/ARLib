#pragma once
#include "Memory.h"
#include "std_includes.h"

namespace ARLib {
    template <typename T, size_t SSO = 15>
    class SSOVector {
        T m_situ_storage[SSO];
        size_t m_capacity = SSO;
        size_t m_size = 0;
        T* m_storage = addressof(m_situ_storage[0]);

        void grow_internal(size_t new_capacity) {
            // if grow internal has been called, it means that we need to grow out of situ
            HARD_ASSERT(new_capacity > m_capacity, "New capacity should be bigger than existing capacity")
            if (m_capacity == SSO) {
                m_storage = new T[new_capacity];
                m_capacity = new_capacity;
                ConditionalBitCopy(m_storage, addressof(m_situ_storage[0]), m_size);
            } else {
                T* new_storage = new T[new_capacity];
                ConditionalBitMove(new_storage, m_storage, m_size);
                delete[] m_storage;
                m_storage = new_storage;
                m_capacity = new_capacity;
            }
        }

        void grow_to_capacity(size_t requested_capacity) { grow_internal(basic_growth(requested_capacity)); }

        void append_internal(T&& item) {
            if (m_size == m_capacity) { grow_to_capacity(m_capacity + 1); }
            m_storage[m_size++] = move(item);
        }

        public:
        constexpr SSOVector() = default;

        template <MoveAssignable... Values>
        SSOVector(T&& val, Values&&... values) requires AllOfV<T, Values...> {
            reserve(sizeof...(values) + 1);
            append(Forward<T>(val));
            (append(Forward<Values>(values)), ...);
        }

        template <size_t OTHER_SSO>
        SSOVector(const SSOVector<T, OTHER_SSO>& other) requires(OTHER_SSO != SSO) : m_size(other.size()) {
            if (other.size() > SSO) {
                m_storage = new T[other.capacity()];
                m_capacity = other.capacity();
            }
            ConditionalBitCopy(m_storage, other.storage(), other.size());
        }
        template <size_t OTHER_SSO>
        SSOVector(SSOVector<T, OTHER_SSO>&& other) requires(OTHER_SSO != SSO) : m_size(other.size()) {
            if (other.size() > SSO && other.capacity() == OTHER_SSO) {
                m_storage = new T[other.capacity()];
                m_capacity = other.capacity();
                ConditionalBitCopy(m_storage, other.storage(), other.size());
            } else if ((OTHER_SSO < SSO && other.capacity() == OTHER_SSO) || other.size() <= SSO) {
                ConditionalBitCopy(m_storage, other.storage(), other.size());
                if (other.capacity() != OTHER_SSO) other.release_strong();
            } else {
                m_capacity = other.capacity();
                m_storage = other.release();
            }
        }

        SSOVector(const SSOVector& other) : m_size(other.m_size), m_capacity(other.m_capacity) {
            if (other.m_capacity != SSO) { m_storage = new T[other.m_capacity]; }
            ConditionalBitCopy(m_storage, other.m_storage, other.m_size);
        }
        SSOVector(SSOVector&& other) noexcept : m_size(other.m_size), m_capacity(other.m_capacity) {
            if (other.m_capacity == SSO) {
                ConditionalBitMove(m_situ_storage, other.m_situ_storage, other.m_size);
            } else {
                m_storage = other.m_storage;
                other.m_storage = addressof(other.m_situ_storage[0]);
                other.m_capacity = SSO;
                other.m_size = 0;
            }
        }

        template <size_t OTHER_SSO>
        SSOVector& operator=(const SSOVector<T, OTHER_SSO>& other) requires(SSO != OTHER_SSO) {
            m_size = other.size();
            if (m_size > m_capacity) {
                if (m_capacity != SSO) delete[] m_storage;
                m_storage = new T[other.capacity()];
                m_capacity = other.capacity();
            }
            ConditionalBitCopy(m_storage, other.storage(), m_size);
            return *this;
        }

        template <size_t OTHER_SSO>
        SSOVector& operator=(SSOVector<T, OTHER_SSO>&& other) requires(SSO != OTHER_SSO) {
            release_strong();
            m_size = other.size();
            if (other.capacity() == OTHER_SSO) {
                if (OTHER_SSO < SSO) {
                    m_capacity = OTHER_SSO;
                    m_storage = new T[OTHER_SSO];
                }
                ConditionalBitCopy(m_storage, other.storage(), m_size);
            } else {
                m_capacity = other.capacity();
                m_storage = other.release();
            }
            return *this;
        }

        SSOVector& operator=(const SSOVector& other) {
            if (this == &other) return *this;
            if (m_capacity > SSO && other.m_size > m_capacity) {
                // if we're already not in situ and we can't fit the other vector, let's resize
                // if we're already not in situ *but* we can fit the other vector, we don't do anything
                delete[] m_storage;
                m_storage = new T[other.m_size];
            } else if (other.m_capacity == SSO && m_capacity > SSO) {
                // if the other one is in situ but we're not, then we delete our storage, since we don't really need it
                delete[] m_storage;
                m_storage = addressof(m_situ_storage[0]);
            }
            m_size = other.m_size;
            m_capacity = other.m_capacity;
            ConditionalBitCopy(m_storage, other.m_storage, other.m_size);
            return *this;
        }
        SSOVector& operator=(SSOVector&& other) noexcept {
            if (this == &other) return *this;
            if (m_capacity > SSO) { delete[] m_storage; }
            m_size = other.m_size;
            m_capacity = other.m_capacity;
            if (other.m_capacity == SSO) {
                m_storage = addressof(m_situ_storage[0]);
                ConditionalBitCopy(m_storage, other.m_storage, other.m_size);
            } else {
                m_storage = other.m_storage;
            }
            other.release_strong();
            return *this;
        }

        consteval auto sso() const { return SSO; }

        T& operator[](size_t index) { return m_storage[index]; }
        const T& operator[](size_t index) const { return m_storage[index]; }

        Iterator<T> begin() { return {m_storage}; }

        ConstIterator<T> begin() const { return {m_storage}; }

        Iterator<T> end() { return {m_storage + m_size}; }

        ConstIterator<T> end() const { return {m_storage + m_size}; }

        bool is_in_situ() { return m_capacity == SSO && m_storage == addressof(m_situ_storage[0]); }

        size_t size() const { return m_size; }
        size_t capacity() const { return m_capacity; }
        const T* storage() const { return m_storage; }
        T* release() {
            HARD_ASSERT(m_capacity != SSO, "Don't release in-situ memory")
            T* released = nullptr;
            if (m_capacity == SSO) {
                released = new T[SSO];
                ConditionalBitCopy(released, m_storage, m_size);
            } else {
                released = m_storage;
                m_storage = addressof(m_situ_storage[0]);
                m_capacity = SSO;
            }
            m_size = 0;
            return released;
        }
        void release_strong() {
            if (m_capacity != SSO) {
                delete[] m_storage;
                m_storage = addressof(m_situ_storage[0]);
                m_capacity = SSO;
            }
            m_size = 0;
        }
        template <typename... Args>
        void emplace_back(Args&&... args) requires Constructible<T, Args...> {
            append_internal(T{Forward<Args>(args)...});
        }
        void push_back(const T& item) {
            T mv{item};
            append_internal(Forward<T>(item));
        }
        void push_back(T&& item) { append_internal(Forward<T>(item)); }
        void append(T&& item) { append_internal(Forward<T>(item)); }
        void resize(size_t new_size) requires DefaultConstructible<T> {
            if (new_size < m_size) return;
            if (new_size > m_capacity) { grow_to_capacity(new_size); }
            for (size_t i = m_size; i < new_size; i++) {
                m_storage[i] = move(T{});
            }
            m_size = new_size;
        }
        void reserve(size_t new_capacity) {
            if (new_capacity < m_capacity) return;
            grow_to_capacity(new_capacity);
        }

        void append(const T& item) {
            T mv{item};
            append_internal(Forward<T>(mv));
        }

        ~SSOVector() {
            if (m_capacity > SSO) delete[] m_storage;
        }
    };
} // namespace ARLib
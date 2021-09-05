#pragma once
#include "Assertion.h"
#include "Concepts.h"

namespace ARLib {
    template <DefaultConstructible T>
    class Optional {
        T* m_object = nullptr;
        bool m_exists = false;

        void assert_not_null_() const { HARD_ASSERT(m_object, "Null deref in Optional") }

        void evict_() {
            if (m_exists) delete m_object;
            m_exists = false;
        }

        public:
        Optional() = default;
        Optional(const Optional& other) {
            if (other.m_exists) { m_object = new T{*other.m_object}; }
            m_exists = other.m_exists;
        }
        Optional(Optional&& other) noexcept {
            m_object = other.m_object;
            m_exists = other.m_exists;
            other.m_object = nullptr;
            other.m_exists = false;
        }

        Optional& operator=(const Optional& other) {
            if (this == &other) return *this;
            if (other.m_exists) *m_object = *other.m_object;
            m_exists = other.m_exists;
            return *this;
        }
        Optional& operator=(Optional&& other) noexcept {
            m_object = other.m_object;
            m_exists = other.m_exists;
            other.m_object = nullptr;
            other.m_exists = false;
            return *this;
        }

        Optional(T&& val) requires MoveAssignable<T> : m_object(new T), m_exists(true) { *m_object = move(val); }

        Optional(const T& val) requires CopyAssignable<T> : m_object(new T), m_exists(true) { *m_object = val; }

        Optional& operator=(const T& val) requires CopyAssignable<T> {
            evict_();
            m_object = new T{val};
            m_exists = true;
            return *this;
        }

        Optional& operator=(T&& val) requires CopyAssignable<T> {
            evict_();
            m_object = new T{Forward<T>(val)};
            m_exists = true;
            return *this;
        }

        bool operator!() { return !m_exists; }
        explicit operator bool() { return m_exists; }

        bool empty() const { return !m_exists; }
        bool has_value() const { return m_exists; };

        const T& value() const& {
            assert_not_null_();
            return *m_object;
        }
        T& value() & {
            assert_not_null_();
            return *m_object;
        }

        T value() && {
            assert_not_null_();
            T val = move(*m_object);
            evict_();
            return val;
        }

        const T* operator->() const {
            assert_not_null_();
            return m_object;
        }

        T* operator->() {
            assert_not_null_();
            return m_object;
        }

        const T& operator*() const {
            assert_not_null_();
            return *m_object;
        }

        T& operator*() {
            assert_not_null_();
            return *m_object;
        }

        T value_or(T&& default_value) && {
            if (!m_exists) return move(default_value);
            T val = move(*m_object);
            evict_();
            return val;
        }

        template <typename... Args>
        void emplace(Args... args) {
            m_object = new T(args...);
            m_exists = true;
        }

        void put(T&& value) {
            if (m_exists) evict_();
            m_object = new T{Forward<T>(value)};
            m_exists = true;
        }

        T* detach() {
            assert_not_null_();
            T* val = m_object;
            m_object = nullptr;
            m_exists = false;
            return val;
        }

        ~Optional() { evict_(); }
    };
} // namespace ARLib

using ARLib::Optional;
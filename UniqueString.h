#pragma once
#include "String.h"
#include "Types.h"
#include "WeakPtr.h"

namespace ARLib {
    class UniqueString {
        WeakPtr<String> m_ref;

        static WeakPtr<String> construct(String s);

        public:
        explicit UniqueString(const String& str) : m_ref(construct(str)) {}
        explicit UniqueString(const char* ptr) : m_ref(construct(String{ptr})) {}

        UniqueString(const UniqueString& other) = default;
        UniqueString(UniqueString&& other) = default;
        UniqueString& operator=(const UniqueString& other) = default;
        UniqueString& operator=(UniqueString&& other) = default;
        UniqueString& operator=(const String& other) {
            m_ref = construct(other);
            return *this;
        }
        UniqueString& operator=(String&& other) {
            m_ref = construct(other);
            return *this;
        }
        bool operator==(const UniqueString& other) const { return m_ref == other.m_ref; }
        bool operator==(const String& other) const { return *m_ref == other; }
        WeakPtr<String> operator->() { return m_ref; }
        const WeakPtr<String> operator->() const { return m_ref; }
    };
} // namespace ARLib
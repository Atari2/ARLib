#pragma once
#include "LinkedSet.h"
#include "SharedPtr.h"
#include "String.h"
#include "Types.h"

namespace ARLib {
    namespace detail {
        static inline LinkedSet<String> s_interned_strings{};
    } // namespace detail

    class UniqueString {
        String* m_ref;

        static String* construct(const String& s) { return &ARLib::detail::s_interned_strings.prepend(s); }

        public:
        UniqueString(const String& str) : m_ref(construct(str)) {  }
        UniqueString(const char* ptr) : m_ref(construct(String{ptr})) {
        }
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
        String* operator->() { return m_ref; }
        const String* operator->() const { return m_ref; }
    };
} // namespace ARLib
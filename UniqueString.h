#pragma once
#include "List.h"
#include "SharedPtr.h"
#include "String.h"
#include "Types.h"

namespace ARLib {
    namespace detail {
        static inline LinkedList<String> s_interned_strings{};
    } // namespace detail

    class UniqueString {
        String* m_ref;

        void construct(const String& s) {
            auto end = detail::s_interned_strings.end();
            auto f = detail::s_interned_strings.find(s);
            if (f == end) {
                detail::s_interned_strings.prepend(s);
                m_ref = &(*detail::s_interned_strings.find(s));
            } else {
                m_ref = &(*f);
            }
        }

        public:
        UniqueString(const String& str) { construct(str); }
        UniqueString(const char* ptr) {
            String s{ptr};
            construct(s);
        }
        UniqueString(const UniqueString& other) = default;
        UniqueString(UniqueString&& other) = default;
        UniqueString& operator=(const UniqueString& other) = default;
        UniqueString& operator=(UniqueString&& other) = default;
        UniqueString& operator=(const String& other) {
            construct(other);
            return *this;
        }
        UniqueString& operator=(String&& other) {
            construct(other);
            return *this;
        }
        bool operator==(const UniqueString& other) const { return m_ref == other.m_ref; }
        bool operator==(const String& other) const { return *m_ref == other; }
        String* operator->() { return m_ref; }
        const String* operator->() const { return m_ref; }
    };
} // namespace ARLib
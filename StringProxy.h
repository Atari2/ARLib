#pragma once
#include "Iterator.h"
#include "Types.h"

namespace ARLib {
    class String;
    class StringView;

    // unsafe lightweight container for non-null terminated strings
    // stringview is "const", while this is mutable
    // basically just a fancy wrapper around char* with a size (avoid strlen is the name of the game)
    class StringProxy {
        char* m_buf = nullptr;
        size_t m_size = 0;

        public:
        StringProxy(StringProxy&&) = default;
        StringProxy(const StringProxy&) = default;
        StringProxy& operator=(const StringProxy&) = default;
        StringProxy& operator=(StringProxy&&) = default;

        StringProxy(String&);
        StringProxy(StringView);
        StringProxy& operator=(String&);
        StringProxy& operator=(StringView);
        size_t size() const { return m_size; }
        Iterator<char> begin() const { return {m_buf}; }
        Iterator<char> end() const { return {m_buf + m_size}; }
        char& operator[](size_t index) const { return m_buf[index]; };
        char* ptr() const { return m_buf; }
    };
} // namespace ARLib

using ARLib::StringProxy;
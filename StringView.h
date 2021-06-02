#pragma once
#include "Iterator.h"
#include "HashBase.h"
#include "String.h"

namespace ARLib {
    class StringView {
        const char* m_start = nullptr;
        const char* m_end = nullptr;

    public:
        static constexpr auto npos = String::npos;
        StringView(const char* buf, size_t size) : m_start(buf) { m_end = buf + size; }
        StringView(const char* buf) : m_start(buf) { m_end = buf + strlen(buf); }
        StringView(const String& ref) : m_start(ref.data()) { m_end = m_start + ref.length(); }
        [[nodiscard]] StringView substring(size_t size) {
            return StringView{ m_start, size };
        }
        [[nodiscard]] size_t size() const { return m_end - m_start; }
        [[nodiscard]] size_t length() const { return m_end - m_start; }
        [[nodiscard]] const char* data() const { return m_start; }
        [[nodiscard]] ConstIterator<char> begin() { return { m_start }; }
        [[nodiscard]] ConstIterator<char> end() { return { m_end }; }
        [[nodiscard]] String extract_string() const { return String(m_start, length()); }
        operator String() const {
            return extract_string();
        }
    };

    StringView operator""_sv(const char* source, size_t len) {
        return StringView{ source, len + 1 };
    }

    template <>
    struct Hash<StringView> {
        [[nodiscard]] size_t operator()(const StringView& key) const noexcept {
            return hash_array_representation(key.data(), key.size());
        }
    };
}

using ARLib::StringView;
using ARLib::operator""_sv;
#pragma once
#include "HashBase.h"
#include "Iterator.h"
#include "String.h"

namespace ARLib {
    // this class is not necessarily null-terminated
    class StringView {
        const char* m_start = nullptr;
        const char* m_end = nullptr;

        public:
        static constexpr auto npos = String::npos;
        constexpr StringView() = default;

        constexpr StringView(const char* begin, const char* end) : m_start(begin), m_end(end) {}

        template <typename T, typename = EnableIfT<IsAnyOfV<T, const char*, char*>>>
        constexpr StringView(T buf, size_t size) : m_start(buf), m_end(buf + size) {}

        template <size_t N>
        consteval StringView(const char (&buf)[N]) : m_start(buf), m_end(buf + N - 1) {}

        template <typename T, typename = EnableIfT<IsAnyOfV<T, const char*, char*>>>
        StringView(T buf) : m_start(buf) {
            m_end = buf + strlen(buf);
        }
        StringView(const String& ref) : m_start(ref.data()) { m_end = m_start + ref.length(); }
        StringView(const StringView& view) : m_start(view.m_start), m_end(view.m_end) {}
        StringView(StringView&& view) noexcept {
            m_start = view.m_start;
            m_end = view.m_end;
            view.m_start = nullptr;
            view.m_end = nullptr;
        }
        StringView& operator=(const StringView& view) noexcept {
            m_start = view.m_start;
            m_end = view.m_end;
            return *this;
        }
        StringView& operator=(StringView&& view) noexcept {
            m_start = view.m_start;
            m_end = view.m_end;
            view.m_start = nullptr;
            view.m_end = nullptr;
            return *this;
        }

        const char& operator[](size_t index) const noexcept { return m_start[index]; }

        [[nodiscard]] StringView substring(size_t size) { return StringView{m_start, size}; }
        void print_view() { printf("%.*s\n", size(), m_start); }
        [[nodiscard]] size_t size() const { return static_cast<size_t>(m_end - m_start); }
        [[nodiscard]] size_t length() const { return static_cast<size_t>(m_end - m_start); }
        [[nodiscard]] const char* data() const { return m_start; }
        // this const_cast is safe because the original buffer is not const
        // String's internal buffers are not const'ed
        [[nodiscard]] char* rawptr() { return const_cast<char*>(m_start); }
        [[nodiscard]] ConstIterator<char> begin() { return {m_start}; }
        [[nodiscard]] ConstIterator<char> end() { return {m_end}; }
        [[nodiscard]] String extract_string() const { return String(m_start, length()); }
        operator String() const { return extract_string(); }
        StringView substringview(size_t first = 0, size_t last = npos) {
            size_t ssize = size();
            if (first > ssize) return {};
            if (last > ssize) return StringView{m_start + first, m_end};
            return StringView{m_start + first, m_start + last};
        }
        bool is_empty() const { return !m_start; }
    };

    constexpr StringView operator""_sv(const char* source, size_t len) { return StringView{source, len}; }

    template <>
    struct Hash<StringView> {
        [[nodiscard]] size_t operator()(const StringView& key) const noexcept {
            return hash_array_representation(key.data(), key.size());
        }
    };
} // namespace ARLib

using ARLib::StringView;
using ARLib::operator""_sv;

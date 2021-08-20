#pragma once
#include "HashBase.h"
#include "Iterator.h"
#include "String.h"

namespace ARLib {
    // this class is not necessarily null-terminated
    class StringView {
        char* m_start_mut = nullptr;
        const char* m_start = nullptr;
        const char* m_end = nullptr;

        public:
        static constexpr auto npos = String::npos;
        constexpr StringView() = default;

        constexpr StringView(const char* begin, const char* end) : m_start(begin), m_end(end) {}
        constexpr StringView(char* begin, const char* end) : m_start_mut(begin), m_start(begin), m_end(end) {}

        template <typename T>
        requires IsSameV<T, const char*>
        constexpr StringView(T buf, size_t size) : m_start(buf), m_end(buf + size) {}

        template <typename T>
        requires IsSameV<T, char*>
        constexpr StringView(T buf, size_t size) : m_start_mut(buf), m_start(buf), m_end(buf + size) {}

        template <size_t N>
        consteval explicit StringView(const char (&buf)[N]) : m_start(buf), m_end(buf + N - 1) {}

        template <typename T>
        requires IsSameV<T, const char*> explicit StringView(T buf) : m_start(buf) { m_end = buf + strlen(buf); }

        template <typename T>
        requires IsSameV<T, char*> explicit StringView(T buf) : m_start_mut(buf), m_start(buf) { m_end = buf + strlen(buf); }

        explicit StringView(const String& ref) : m_start(ref.data()) { m_end = m_start + ref.length(); }
        explicit StringView(String& ref) : m_start_mut(ref.rawptr()), m_start(ref.data()) { m_end = m_start + ref.length(); }
        StringView(const StringView& view) = default;
        StringView(StringView&& view) noexcept {
            m_start_mut = view.m_start_mut;
            m_start = view.m_start;
            m_end = view.m_end;
            view.m_start = nullptr;
            view.m_end = nullptr;
        }
        StringView& operator=(const StringView& view) noexcept {
            if (this == &view) return *this;
            m_start_mut = view.m_start_mut;
            m_start = view.m_start;
            m_end = view.m_end;
            return *this;
        }
        StringView& operator=(StringView&& view) noexcept {
            m_start_mut = view.m_start_mut;
            m_start = view.m_start;
            m_end = view.m_end;
            view.m_start_mut = nullptr;
            view.m_start = nullptr;
            view.m_end = nullptr;
            return *this;
        }

        bool operator==(const StringView& view) const {
            size_t sz = size();
            if (view.size() != sz) return false;
            return strncmp(m_start, view.m_start, sz) == 0;
        }

        bool operator!=(const StringView& view) const {
            size_t sz = size();
            if (view.size() != sz) return true;
            return strncmp(m_start, view.m_start, sz) != 0;
        }

        [[nodiscard]] Ordering operator<=>(const StringView& other) const;

        char operator[](size_t index) const noexcept { return m_start[index]; }
        const char& index(size_t index) const noexcept { return m_start[index]; }
        char& index(size_t index) noexcept {
            HARD_ASSERT(m_start_mut, "This stringview is not mutable")
            return m_start_mut[index];
        }

        [[nodiscard]] StringView substring(size_t size) {
            if (m_start_mut) {
                return StringView{m_start_mut, size};
            } else {
                return StringView{m_start, size};
            }
        }
        void print_view() { printf("%.*s\n", size(), m_start); }
        [[nodiscard]] size_t size() const { return static_cast<size_t>(m_end - m_start); }
        [[nodiscard]] size_t length() const { return static_cast<size_t>(m_end - m_start); }
        [[nodiscard]] const char* data() const { return m_start; }
        [[nodiscard]] char* rawptr() {
            HARD_ASSERT(m_start_mut, "This stringview is not mutable")
            return m_start_mut;
        }
        [[nodiscard]] ConstIterator<char> begin() { return ConstIterator<char>{m_start}; }
        [[nodiscard]] ConstIterator<char> end() { return ConstIterator<char>{m_end}; }
        [[nodiscard]] String extract_string() const { return {m_start, length()}; }
        explicit operator String() const { return extract_string(); }
        StringView substringview(size_t first = 0, size_t last = npos) {
            size_t ssize = size();
            if (first > ssize) return {};
            if (last > ssize) return StringView{m_start + first, m_end};
            if (m_start_mut) {
                return StringView{m_start_mut + first, m_start + last};
            } else {
                return StringView{m_start + first, m_start + last};
            }
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

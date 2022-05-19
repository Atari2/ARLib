#pragma once
#include "HashBase.h"
#include "Iterator.h"
#include "PrintInfo.h"
#include "String.h"

namespace ARLib {

    template <typename T>
    class Vector;

    // this class is not necessarily null-terminated
    class StringView {
        char* m_start_mut = nullptr;
        const char* m_start = nullptr;
        size_t m_size = 0;

        public:
        static constexpr auto npos = String::npos;
        constexpr StringView() = default;

        constexpr StringView(const char* begin, const char* end) :
            m_start(begin), m_size(static_cast<size_t>(end - begin)) {}
        constexpr StringView(char* begin, const char* end) :
            m_start_mut(begin), m_start(begin), m_size(static_cast<size_t>(end - begin)) {}

        template <typename T>
        requires IsSameV<T, const char*>
        constexpr StringView(T buf, size_t size) : m_start(buf), m_size(size) {}

        template <typename T>
        requires IsSameV<T, char*>
        constexpr StringView(T buf, size_t size) : m_start_mut(buf), m_start(buf), m_size(size) {}

        template <size_t N>
        constexpr StringView(const char (&buf)[N]) : m_start(buf), m_size(N - 1) {}

        template <typename T>
        requires IsSameV<T, const char*>
        constexpr StringView(T buf) : m_start(buf) { m_size = strlen(buf); }

        template <typename T>
        requires IsSameV<T, char*>
        constexpr StringView(T buf) : m_start_mut(buf), m_start(buf) { m_size = strlen(buf); }

        explicit StringView(const String& ref) : m_start(ref.data()) { m_size = ref.length(); }
        explicit StringView(String& ref) : m_start_mut(ref.rawptr()), m_start(ref.data()) { m_size = ref.length(); }
        constexpr StringView(const StringView& view) = default;
        constexpr StringView(StringView&& view) noexcept {
            m_start_mut = view.m_start_mut;
            m_start = view.m_start;
            m_size = view.m_size;
            view.m_start = nullptr;
            view.m_size = 0;
        }
        constexpr StringView& operator=(const StringView& view) noexcept {
            if (this == &view) return *this;
            m_start_mut = view.m_start_mut;
            m_start = view.m_start;
            m_size = view.m_size;
            return *this;
        }
        constexpr StringView& operator=(StringView&& view) noexcept {
            m_start_mut = view.m_start_mut;
            m_start = view.m_start;
            m_size = view.m_size;
            view.m_start_mut = nullptr;
            view.m_start = nullptr;
            view.m_size = 0;
            return *this;
        }

        template <size_t N>
        constexpr bool operator==(const char (&buf)[N]) const noexcept {
            size_t sz = size();
            if (sz != (N - 1)) return false;
            return strncmp(m_start, buf, N - 1) == 0;
        }

        template <typename T>
        requires IsAnyOfV<T, char*, const char*>
        constexpr bool operator==(T buf) const noexcept {
            size_t sz = size();
            size_t o_sz = strlen(buf);
            if (o_sz != sz) return false;
            return strncmp(m_start, buf, o_sz) == 0;
        }

        constexpr bool operator==(const StringView& view) const {
            size_t sz = size();
            if (view.size() != sz) return false;
            return strncmp(m_start, view.m_start, sz) == 0;
        }

        constexpr bool operator!=(const StringView& view) const {
            size_t sz = size();
            if (view.size() != sz) return true;
            return strncmp(m_start, view.m_start, sz) != 0;
        }

        [[nodiscard]] Ordering operator<=>(const StringView& other) const;

        constexpr char operator[](size_t index) const noexcept { return m_start[index]; }
        constexpr const char& index(size_t index) const noexcept { return m_start[index]; }
        constexpr char& index(size_t index) noexcept {
            HARD_ASSERT(m_start_mut, "This stringview is not mutable")
            return m_start_mut[index];
        }

        [[nodiscard]] constexpr StringView substring(size_t size) {
            if (m_start_mut) {
                return StringView{m_start_mut, size};
            } else {
                return StringView{m_start, size};
            }
        }

        [[nodiscard]] size_t index_of(const char* c, size_t start = 0) const;
        Vector<StringView> split(const char* sep = " ") const;

        void print_view() { printf("%.*s\n", size(), m_start); }
        [[nodiscard]] constexpr size_t size() const { return m_size; }
        [[nodiscard]] constexpr size_t length() const { return m_size; }
        [[nodiscard]] constexpr const char* data() const { return m_start; }
        [[nodiscard]] constexpr char* rawptr() {
            HARD_ASSERT(m_start_mut, "This stringview is not mutable")
            return m_start_mut;
        }
        [[nodiscard]] ConstIterator<char> begin() { return ConstIterator<char>{m_start}; }
        [[nodiscard]] ConstIterator<char> end() { return ConstIterator<char>{m_start + m_size}; }
        [[nodiscard]] String extract_string() const { return {m_start, length()}; }
        explicit operator String() const { return extract_string(); }
        constexpr StringView substringview(size_t first = 0, size_t last = npos) const {
            size_t ssize = size();
            if (first > ssize) return {};
            if (last > ssize) return StringView{m_start + first, m_size - first};
            if (m_start_mut) {
                return StringView{m_start_mut + first, m_start + last};
            } else {
                return StringView{m_start + first, m_start + last};
            }
        }
        constexpr bool is_empty() const { return !m_start; }
    };

    constexpr StringView operator""_sv(const char* source, size_t len) { return StringView{source, len}; }

    template <>
    struct Hash<StringView> {
        [[nodiscard]] size_t operator()(const StringView& key) const noexcept {
            return hash_array_representation(key.data(), key.size());
        }
    };

    template <>
    struct PrintInfo<StringView> {
        const StringView& m_view;
        PrintInfo(const StringView& view) : m_view(view) {}
        String repr() const { return m_view.extract_string(); }
    };
} // namespace ARLib

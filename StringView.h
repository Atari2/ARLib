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
    char* m_start_mut   = nullptr;
    const char* m_start = nullptr;
    size_t m_size       = 0;

    public:
    constexpr static auto npos = String::npos;
    constexpr StringView()     = default;
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
    constexpr StringView(const char (&buf)[N]) : m_start(buf), m_size(strlen(buf)) {}
    template <typename T>
    requires IsSameV<T, const char*>
    constexpr StringView(T buf) : m_start(buf) {
        m_size = strlen(buf);
    }
    template <typename T>
    requires IsSameV<T, char*>
    constexpr StringView(T buf) : m_start_mut(buf), m_start(buf) {
        m_size = strlen(buf);
    }
    explicit StringView(const String& ref) : m_start(ref.data()) { m_size = ref.length(); }
    explicit StringView(String& ref) : m_start_mut(ref.rawptr()), m_start(ref.data()) { m_size = ref.length(); }
    constexpr StringView(const StringView& view) = default;
    constexpr StringView(StringView&& view) noexcept {
        m_start_mut  = view.m_start_mut;
        m_start      = view.m_start;
        m_size       = view.m_size;
        view.m_start = nullptr;
        view.m_size  = 0;
    }
    constexpr StringView& operator=(const StringView& view) noexcept {
        if (this == &view) return *this;
        m_start_mut = view.m_start_mut;
        m_start     = view.m_start;
        m_size      = view.m_size;
        return *this;
    }
    constexpr StringView& operator=(StringView&& view) noexcept {
        m_start_mut      = view.m_start_mut;
        m_start          = view.m_start;
        m_size           = view.m_size;
        view.m_start_mut = nullptr;
        view.m_start     = nullptr;
        view.m_size      = 0;
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
        size_t sz   = size();
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
    constexpr bool empty() const { return size() == 0; }
    [[nodiscard]] Ordering operator<=>(const StringView& other) const;
    constexpr char operator[](size_t index) const noexcept { return m_start[index]; }
    constexpr const char& index(size_t index) const noexcept { return m_start[index]; }
    constexpr char& index(size_t index) noexcept {
        HARD_ASSERT(m_start_mut, "This stringview is not mutable")
        return m_start_mut[index];
    }
    [[nodiscard]] constexpr StringView substring(size_t size) {
        if (m_start_mut) {
            return StringView{ m_start_mut, size };
        } else {
            return StringView{ m_start, size };
        }
    }
    [[nodiscard]] constexpr size_t index_of(const char* c, size_t start = 0) const {
        if (m_size == 0 || start >= m_size) return npos;
        const char* buf = m_start;
        auto o_len      = strlen(c);
        if (o_len > m_size) return npos;
        if (start + o_len > m_size) return npos;
        if (o_len == m_size && start == 0 && strcmp(buf, c) == 0) return 0;
        for (size_t i = start; i < m_size; i++) {
            if (strncmp(buf + i, c, o_len) == 0) return i;
        }
        return npos;
    }
    Vector<StringView> split(const char* sep = " ") const;
    void print_view() { printf("%.*s\n", size(), m_start); }
    [[nodiscard]] constexpr size_t size() const { return m_size; }
    [[nodiscard]] constexpr size_t length() const { return m_size; }
    [[nodiscard]] constexpr const char* data() const { return m_start; }
    [[nodiscard]] constexpr char* rawptr() {
        HARD_ASSERT(m_start_mut, "This stringview is not mutable")
        return m_start_mut;
    }
    [[nodiscard]] constexpr ConstIterator<char> begin() const { return ConstIterator<char>{ m_start }; }
    [[nodiscard]] constexpr ConstIterator<char> end() const { return ConstIterator<char>{ m_start + m_size }; }
    [[nodiscard]] String extract_string() const { return { m_start, length() }; }
    explicit operator String() const { return extract_string(); }
    constexpr StringView substringview(size_t first = 0, size_t last = npos) const {
        size_t ssize = size();
        if (first > ssize) return {};
        if (last > ssize) return StringView{ m_start + first, m_size - first };
        if (m_start_mut) {
            return StringView{ m_start_mut + first, m_start + last };
        } else {
            return StringView{ m_start + first, m_start + last };
        }
    }
    constexpr StringView substringview_fromlen(size_t first = 0, size_t len = npos) const {
        const size_t rcount = min_bt(len, size() - first);
        return StringView{ data() + first, rcount };
    }
    [[nodiscard]] constexpr size_t index_of(char c, size_t off = 0) const {
        const char* ptr = data();
        if (off > m_size) return npos;
        for (size_t i = off; i < m_size; i++) {
            if (ptr[i] == c) return i;
        }
        return npos;
    }
    [[nodiscard]] constexpr size_t index_not_of(char c, size_t off = 0) const {
        const char* ptr = data();
        if (off > m_size) return npos;
        for (size_t i = off; i < m_size; i++) {
            if (ptr[i] != c) return i;
        }
        return npos;
    }
    constexpr bool is_empty() const { return !m_start; }
};
constexpr StringView operator""_sv(const char* source, size_t len) {
    return StringView{ source, len };
}
template <>
struct Hash<StringView> {
    [[nodiscard]] size_t operator()(const StringView& key) const noexcept {
        constexpr size_t seed = static_cast<size_t>(0xc70f6907UL);
        return murmur_hash_bytes(key.data(), key.size(), seed);
    }
};
template <>
struct PrintInfo<StringView> {
    const StringView& m_view;
    PrintInfo(const StringView& view) : m_view(view) {}
    String repr() const { return m_view.extract_string(); }
};
}    // namespace ARLib

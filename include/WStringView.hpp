#pragma once
#include "HashBase.hpp"
#include "Iterator.hpp"
#include "PrintInfo.hpp"
#include "WString.hpp"
#include "Memory.hpp"
namespace ARLib {

template <typename T>
class Vector;
// this class is not necessarily null-terminated
class WStringView {
    wchar_t* m_start_mut   = nullptr;
    const wchar_t* m_start = nullptr;
    size_t m_size          = 0;

    public:
    constexpr static auto npos = String::npos;
    constexpr WStringView()    = default;
    constexpr WStringView(const wchar_t* begin, const wchar_t* end) :
        m_start(begin), m_size(static_cast<size_t>(end - begin)) {}
    constexpr WStringView(wchar_t* begin, const wchar_t* end) :
        m_start_mut(begin), m_start(begin), m_size(static_cast<size_t>(end - begin)) {}
    template <typename T>
    requires IsSameV<T, const wchar_t*>
    constexpr WStringView(T buf, size_t size) : m_start(buf), m_size(size) {}
    template <typename T>
    requires IsSameV<T, wchar_t*>
    constexpr WStringView(T buf, size_t size) : m_start_mut(buf), m_start(buf), m_size(size) {}
    template <size_t N>
    constexpr WStringView(const wchar_t (&buf)[N]) : m_start(buf), m_size(wstrlen(buf)) {}
    template <typename T>
    requires IsSameV<T, const wchar_t*>
    constexpr WStringView(T buf) : m_start(buf) {
        m_size = wstrlen(buf);
    }
    template <typename T>
    requires IsSameV<T, wchar_t*>
    constexpr WStringView(T buf) : m_start_mut(buf), m_start(buf) {
        m_size = wstrlen(buf);
    }
    explicit WStringView(const WString& ref) : m_start(ref.data()) { m_size = ref.length(); }
    explicit WStringView(WString& ref) : m_start_mut(ref.rawptr()), m_start(ref.data()) { m_size = ref.length(); }
    constexpr WStringView(const WStringView& view) = default;
    constexpr WStringView(WStringView&& view) noexcept {
        m_start_mut  = view.m_start_mut;
        m_start      = view.m_start;
        m_size       = view.m_size;
        view.m_start = nullptr;
        view.m_size  = 0;
    }
    constexpr WStringView& operator=(const WStringView& view) noexcept {
        if (this == &view) return *this;
        m_start_mut = view.m_start_mut;
        m_start     = view.m_start;
        m_size      = view.m_size;
        return *this;
    }
    constexpr WStringView& operator=(WStringView&& view) noexcept {
        m_start_mut      = view.m_start_mut;
        m_start          = view.m_start;
        m_size           = view.m_size;
        view.m_start_mut = nullptr;
        view.m_start     = nullptr;
        view.m_size      = 0;
        return *this;
    }
    template <size_t N>
    constexpr bool operator==(const wchar_t (&buf)[N]) const noexcept {
        size_t sz = size();
        if (sz != (N - 1)) return false;
        return wstrncmp(m_start, buf, N - 1) == 0;
    }
    template <typename T>
    requires IsAnyOfV<T, wchar_t*, const wchar_t*>
    constexpr bool operator==(T buf) const noexcept {
        size_t sz   = size();
        size_t o_sz = wstrlen(buf);
        if (o_sz != sz) return false;
        return wstrncmp(m_start, buf, o_sz) == 0;
    }
    constexpr bool operator==(const WStringView& view) const {
        size_t sz = size();
        if (view.size() != sz) return false;
        return wstrncmp(m_start, view.m_start, sz) == 0;
    }
    constexpr bool operator!=(const WStringView& view) const {
        size_t sz = size();
        if (view.size() != sz) return true;
        return wstrncmp(m_start, view.m_start, sz) != 0;
    }
    constexpr bool empty() const { return size() == 0; }
    [[nodiscard]] Ordering operator<=>(const WStringView& other) const;
    constexpr wchar_t operator[](size_t index) const noexcept { return m_start[index]; }
    constexpr const wchar_t& index(size_t index) const noexcept { return m_start[index]; }
    constexpr wchar_t& index(size_t index) noexcept {
        HARD_ASSERT(m_start_mut, "This WStringView is not mutable")
        return m_start_mut[index];
    }
    [[nodiscard]] constexpr WStringView substring(size_t size) {
        if (m_start_mut) {
            return WStringView{ m_start_mut, size };
        } else {
            return WStringView{ m_start, size };
        }
    }
    [[nodiscard]] bool starts_with(WStringView other) const {
        auto o_len = other.size();
        if (o_len > m_size) return false;
        if (m_size == o_len) return wstrcmp(other.data(), m_start) == 0;
        auto res = wstrncmp(other.data(), m_start, o_len);
        return res == 0;
    }
    [[nodiscard]] constexpr size_t index_of(const wchar_t* c, size_t start = 0) const {
        if (m_size == 0 || start >= m_size) return npos;
        const wchar_t* buf = m_start;
        auto o_len         = wstrlen(c);
        if (o_len > m_size) return npos;
        if (start + o_len > m_size) return npos;
        if (o_len == m_size && start == 0 && wstrcmp(buf, c) == 0) return 0;
        for (size_t i = start; i < m_size; i++) {
            if (wstrncmp(buf + i, c, o_len) == 0) return i;
        }
        return npos;
    }
    Vector<WStringView> split(const wchar_t* sep = L" ") const;
    void print_view() { printf("%.*s\n", size(), m_start); }
    [[nodiscard]] constexpr size_t size() const { return m_size; }
    [[nodiscard]] constexpr size_t length() const { return m_size; }
    [[nodiscard]] constexpr const wchar_t* data() const { return m_start; }
    [[nodiscard]] constexpr wchar_t* rawptr() {
        HARD_ASSERT(m_start_mut, "This WStringView is not mutable")
        return m_start_mut;
    }
    [[nodiscard]] constexpr ConstIterator<wchar_t> begin() const { return ConstIterator<wchar_t>{ m_start }; }
    [[nodiscard]] constexpr ConstIterator<wchar_t> end() const { return ConstIterator<wchar_t>{ m_start + m_size }; }
    [[nodiscard]] WString extract_string() const { return { m_start, length() }; }
    explicit operator WString() const { return extract_string(); }
    constexpr WStringView substringview(size_t first = 0, size_t last = npos) const {
        size_t ssize = size();
        if (first > ssize) return {};
        if (last > ssize) return WStringView{ m_start + first, m_size - first };
        if (m_start_mut) {
            return WStringView{ m_start_mut + first, m_start + last };
        } else {
            return WStringView{ m_start + first, m_start + last };
        }
    }
    constexpr WStringView substringview_fromlen(size_t first = 0, size_t len = npos) const {
        const size_t rcount = min_bt(len, size() - first);
        return WStringView{ data() + first, rcount };
    }
    [[nodiscard]] constexpr size_t index_of(wchar_t c, size_t off = 0) const {
        const wchar_t* ptr = data();
        if (off > m_size) return npos;
        for (size_t i = off; i < m_size; i++) {
            if (ptr[i] == c) return i;
        }
        return npos;
    }
    [[nodiscard]] constexpr size_t index_not_of(wchar_t c, size_t off = 0) const {
        const wchar_t* ptr = data();
        if (off > m_size) return npos;
        for (size_t i = off; i < m_size; i++) {
            if (ptr[i] != c) return i;
        }
        return npos;
    }
    constexpr bool is_empty() const { return !m_start; }
};
constexpr WStringView operator""_wsv(const wchar_t* source, size_t len) {
    return WStringView{ source, len };
}
template <>
struct Hash<WStringView> {
    [[nodiscard]] size_t operator()(const WStringView& key) const noexcept {
        return hash_array_representation(key.data(), key.size());
    }
};
template <>
struct PrintInfo<WStringView> {
    const WStringView& m_view;
    PrintInfo(const WStringView& view) : m_view(view) {}
    String repr() const;
};
}    // namespace ARLib

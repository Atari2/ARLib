#pragma once
#include "Algorithm.hpp"
#include "Allocator.hpp"
#include "Assertion.hpp"
#include "BaseTraits.hpp"
#include "Iterator.hpp"
#include "Types.hpp"
#include "cstring_compat.hpp"
#include "Memory.hpp"
#include "PrintInfo.hpp"
namespace ARLib {
class WStringView;
class Ordering;
template <typename T>
class Vector;
class String;
class StringView;
class WString {
    constexpr static size_t SMALL_STRING_CAP = 15;
    union {
        wchar_t m_local_buf[SMALL_STRING_CAP + 1] = { 0 };
        size_t m_allocated_capacity;
    };
    wchar_t* m_data_buf = PointerTraits<wchar_t*>::pointer_to(*m_local_buf);
    size_t m_size       = 0;
    constexpr wchar_t* data_internal() { return m_data_buf; }
    constexpr const wchar_t* data_internal() const { return m_data_buf; }
    constexpr wchar_t* local_data_internal() { return PointerTraits<wchar_t*>::pointer_to(*m_local_buf); }
    constexpr const wchar_t* local_data_internal() const {
        return PointerTraits<const wchar_t*>::pointer_to(*m_local_buf);
    }
    constexpr bool is_local() const { return data_internal() == local_data_internal(); }
    constexpr const wchar_t* get_buf_internal() const { return data_internal(); }
    constexpr wchar_t* get_buf_internal() { return data_internal(); }
    void grow_internal(size_t requested_capacity) {
        if (is_local()) {
            // grow outside of locality, copy buffer and change active member of union
            requested_capacity = basic_growth(requested_capacity);
            m_data_buf         = allocate_initialized<wchar_t>(requested_capacity);
            ConditionalBitCopy(m_data_buf, m_local_buf, SMALL_STRING_CAP + 1);
            m_allocated_capacity = requested_capacity;
        } else {
            m_allocated_capacity = basic_growth(requested_capacity);
            HARD_ASSERT(
            m_allocated_capacity >= requested_capacity && m_allocated_capacity > m_size, "Allocated capacity failure"
            )
            wchar_t* new_buf = allocate_initialized<wchar_t>(m_allocated_capacity);
            new_buf[m_size]  = L'\0';
            if (m_size != 0) ConditionalBitCopy(new_buf, m_data_buf, m_size + 1);
            deallocate<wchar_t, DeallocType::Multiple>(m_data_buf);
            m_data_buf = new_buf;
        }
    }
    constexpr void grow_if_needed(size_t newsize) {
        if (is_local()) {
            if (newsize > SMALL_STRING_CAP) grow_internal(newsize + 1);
        } else {
            if (newsize > m_allocated_capacity - 1 || m_allocated_capacity == 0) { grow_internal(newsize + 1); }
        }
    }
    Vector<size_t> all_indexes_internal(WStringView any, size_t start_index = 0ull) const;
    Vector<size_t> all_last_indexes_internal(WStringView any, size_t end_index = npos) const;
    Vector<size_t> all_not_indexes_internal(WStringView any, size_t start_index = 0ull) const;
    Vector<size_t> all_last_not_indexes_internal(WStringView any, size_t end_index = npos) const;

    public:
    constexpr static auto npos = static_cast<size_t>(-1);

    // constructors, destructor equality operators
    constexpr WString() = default;
    template <size_t N>
    explicit constexpr WString(const wchar_t (&src)[N]) : m_size(wstrlen(src)) {
        grow_if_needed(m_size);
        ConditionalBitCopy(m_data_buf, src, m_size);
        m_data_buf[m_size] = L'\0';
    }
    explicit WString(size_t size, wchar_t c) {
        if (size > SMALL_STRING_CAP) grow_if_needed(size);
        wchar_t* buf = get_buf_internal();
        for (size_t i = 0; i < size; ++i) { buf[i] = c; }
        m_size      = size;
        buf[m_size] = L'\0';
    }
    explicit constexpr WString(const wchar_t* begin, const wchar_t* end) {
        HARD_ASSERT_FMT((end >= begin), "End pointer (%p) must not be before begin pointer (%p)", end, begin)
        m_size = static_cast<size_t>(end - begin);
        grow_if_needed(m_size);
        ConditionalBitCopy(m_data_buf, begin, m_size);
        m_data_buf[m_size] = L'\0';
    }
    constexpr WString(const wchar_t* other, size_t size) : m_size(size) {
        grow_if_needed(size);
        ConditionalBitCopy(m_data_buf, other, size);
        m_data_buf[m_size] = L'\0';
    }
    template <typename T>
    requires(SameAs<const wchar_t*, T> || SameAs<wchar_t*, T>)
    explicit constexpr WString(T other) : m_size(wstrlen(other)) {
        grow_if_needed(m_size);
        ConditionalBitCopy(m_data_buf, other, m_size);
        m_data_buf[m_size] = L'\0';
    }
    WString(const String& other);
    WString(StringView other);
    WString(const WString& other) noexcept : m_size(other.m_size) {
        grow_if_needed(m_size);
        ConditionalBitCopy(m_data_buf, other.m_data_buf, m_size + 1);
        m_data_buf[m_size] = L'\0';
    }
    WString(WString&& other) noexcept : m_size(other.m_size) {
        if (other.is_local()) {
            ConditionalBitCopy(m_local_buf, other.m_local_buf, SMALL_STRING_CAP + 1);
        } else {
            m_allocated_capacity       = other.m_allocated_capacity;
            m_size                     = other.m_size;
            m_data_buf                 = other.m_data_buf;
            other.m_allocated_capacity = SMALL_STRING_CAP;
        }
        other.m_data_buf = PointerTraits<wchar_t*>::pointer_to(*other.m_local_buf);
        other.m_size     = 0;
    }
    explicit WString(WStringView other);
    WString& operator=(const WString& other) {
        if (this != &other) {
            if (!is_local()) deallocate<wchar_t, DeallocType::Multiple>(m_data_buf);
            m_size = other.m_size;
            grow_if_needed(m_size);
            ConditionalBitCopy(m_data_buf, other.m_data_buf, m_size + 1);
        }
        return *this;
    }
    WString& operator=(WString&& other) noexcept {
        if (this != &other) {
            if (!is_local()) deallocate<wchar_t, DeallocType::Multiple>(m_data_buf);
            m_size = other.m_size;
            if (other.is_local()) {
                ConditionalBitCopy(m_local_buf, other.m_local_buf, SMALL_STRING_CAP + 1);
                m_data_buf = PointerTraits<wchar_t*>::pointer_to(*m_local_buf);
            } else {
                m_data_buf                 = other.m_data_buf;
                m_allocated_capacity       = other.m_allocated_capacity;
                other.m_data_buf           = PointerTraits<wchar_t*>::pointer_to(*other.m_local_buf);
                other.m_size               = 0;
                other.m_allocated_capacity = SMALL_STRING_CAP;
            }
        }
        return *this;
    }
    ~WString() {
        m_size = 0;
        if (!is_local()) {
            deallocate<wchar_t, DeallocType::Multiple>(m_data_buf);
            m_data_buf = nullptr;
        }
    }
    // releases the inner wchar_t* buffer. May allocate if buffer is in-situ. May return nullptr if the WString is
    // empty.
    wchar_t* release() {
        if (m_size == 0) return nullptr;
        if (is_local()) {
            wchar_t* buffer = allocate_initialized<wchar_t>(SMALL_STRING_CAP + 1);
            ConditionalBitCopy(buffer, local_data_internal(), m_size);
            return buffer;
        } else {
            wchar_t* buffer = m_data_buf;
            m_data_buf      = local_data_internal();
            m_size          = 0;
            return buffer;
        }
    }
    [[nodiscard]] WString substring(size_t first = 0, size_t last = npos) const {
        if (last == npos || last > length()) last = length();
        return WString{ get_buf_internal() + first, get_buf_internal() + last };
    }
    [[nodiscard]] WStringView view();
    [[nodiscard]] WStringView view() const;
    [[nodiscard]] WStringView substringview(size_t first = 0, size_t last = npos) const;
    // comparison operators
    [[nodiscard]] bool operator==(const WString& other) const {
        if (other.m_size == m_size) { return wstrncmp(m_data_buf, other.m_data_buf, m_size) == 0; }
        return false;
    }
    template <typename T, typename = EnableIfT<IsAnyOfV<T, const wchar_t*, wchar_t*>>>
    [[nodiscard]] bool operator==(T other) const {
        return wstrcmp(get_buf_internal(), other) == 0;
    }
    template <typename T, typename = EnableIfT<IsAnyOfV<T, const wchar_t*, wchar_t*>>>
    [[nodiscard]] bool operator!=(T other) const {
        return wstrcmp(get_buf_internal(), other) != 0;
    }
    template <size_t N>
    [[nodiscard]] bool operator==(const wchar_t (&other)[N]) const {
        if (N - 1 != m_size) return false;
        return wstrncmp(get_buf_internal(), other, N - 1) == 0;
    }
    template <size_t N>
    [[nodiscard]] bool operator!=(const wchar_t (&other)[N]) const {
        if (N - 1 != m_size) return true;
        return wstrncmp(get_buf_internal(), other, N - 1) != 0;
    }
    [[nodiscard]] bool operator==(const WStringView& other) const;
    [[nodiscard]] bool operator!=(const WStringView& other) const;
    [[nodiscard]] bool operator<(const WString& other) const {
        return wstrncmp(get_buf_internal(), other.get_buf_internal(), other.m_size) < 0;
    }
    [[nodiscard]] bool operator<(const WStringView& other) const;
    [[nodiscard]] bool operator>(const WString& other) const { return !(*this < other) && !(*this == other); }
    [[nodiscard]] bool operator<=(const WString& other) const { return (*this < other || *this == other); }
    [[nodiscard]] bool operator>=(const WString& other) const { return (*this > other || *this == other); }
    [[nodiscard]] Ordering operator<=>(const WString& other) const;
    [[nodiscard]] Ordering operator<=>(const WStringView& other) const;
    void set_size(size_t size) {
        m_size                     = size;
        get_buf_internal()[m_size] = L'\0';
    }
    void resize(size_t size) {
        reserve(size);
        set_size(size);
    }
    [[nodiscard]] size_t size() const { return m_size; }
    [[nodiscard]] size_t length() const { return m_size; }
    [[nodiscard]] size_t capacity() const { return is_local() ? SMALL_STRING_CAP : m_allocated_capacity; }
    [[nodiscard]] const wchar_t* data() const { return get_buf_internal(); }
    [[nodiscard]] wchar_t* rawptr() { return get_buf_internal(); }
    [[nodiscard]] bool is_empty() const { return m_size == 0; }
    void clear() { set_size(0); }
    // starts/ends with
    [[nodiscard]] bool starts_with(const WString& other) const {
        if (other.m_size > m_size) return false;
        if (other.m_size == m_size) return other == *this;
        auto res = wstrncmp(other.get_buf_internal(), get_buf_internal(), other.m_size);
        return res == 0;
    }
    [[nodiscard]] bool starts_with(WStringView) const;
    [[nodiscard]] bool ends_with(const WString& other) const {
        if (other.m_size > m_size) return false;
        if (other.m_size == m_size) return other == *this;
        auto ptrdiff          = m_size - other.m_size;
        const wchar_t* buf    = other.get_buf_internal();
        const wchar_t* my_buf = get_buf_internal();
        auto res              = wstrncmp(my_buf + ptrdiff, buf, other.m_size);
        return res == 0;
    }
    [[nodiscard]] bool ends_with(WStringView other) const;
    // concatenation
    void append(wchar_t c) {
        auto new_size = m_size + 1;
        grow_if_needed(new_size);
        get_buf_internal()[m_size] = c;
        set_size(m_size + 1);
    }
    void append(const WString& other) {
        auto new_size = m_size + other.m_size;
        grow_if_needed(new_size);
        wstrcat_eff(get_buf_internal() + m_size, other.get_buf_internal());
        set_size(m_size + other.m_size);
    }
    void append(WStringView other);
    void append(const wchar_t* other);
    [[nodiscard]] WString concat(wchar_t c) const {
        WString copy{ *this };
        auto new_size = m_size + 1;
        copy.grow_if_needed(new_size);
        copy.get_buf_internal()[m_size] = c;
        copy.set_size(m_size + 1);
        return copy;
    }
    [[nodiscard]] WString concat(const WString& other) const {
        WString copy{ *this };
        auto new_size = m_size + other.m_size;
        copy.grow_if_needed(new_size);
        wstrcat_eff(copy.get_buf_internal() + m_size, other.get_buf_internal());
        copy.set_size(new_size);
        return copy;
    }
    [[nodiscard]] WString concat(WStringView other) const;
    [[nodiscard]] WString concat(const wchar_t* other) const;
    WString operator+(const WString& other) const { return concat(other); }
    WString operator+(const wchar_t* other) const { return concat(other); }
    WString operator+(wchar_t c) const { return concat(c); }
    WString& operator+=(const WString& other) {
        append(other);
        return *this;
    }
    WString& operator+=(const wchar_t* other) {
        append(other);
        return *this;
    }
    WString& operator+=(wchar_t c) {
        append(c);
        return *this;
    }
    // reverse
    WString reversed() {
        WString reversed{};
        reversed.reserve(size());
        for (auto rc = this->rbegin(); rc != this->rend(); rc--) reversed.append(*rc);
        return reversed;
    }
    // iterator support
    [[nodiscard]] Iterator<wchar_t> begin() { return Iterator<wchar_t>{ get_buf_internal() }; }
    [[nodiscard]] ConstIterator<wchar_t> begin() const { return ConstIterator<wchar_t>{ get_buf_internal() }; }
    [[nodiscard]] Iterator<wchar_t> rbegin() { return end() - 1; }
    [[nodiscard]] Iterator<wchar_t> end() { return Iterator<wchar_t>{ get_buf_internal() + m_size }; }
    [[nodiscard]] ConstIterator<wchar_t> end() const { return ConstIterator<wchar_t>{ get_buf_internal() + m_size }; }
    [[nodiscard]] Iterator<wchar_t> rend() { return begin() - 1; }
    [[nodiscard]] wchar_t front() const { return get_buf_internal()[0]; }
    [[nodiscard]] wchar_t back() const { return get_buf_internal()[m_size - 1]; }
    // indexing access
    [[nodiscard]] wchar_t at(size_t index) const {
        SOFT_ASSERT_FMT((index >= m_size), "Index of %llu was out of bounds of WString with size %llu", index, m_size)
        return get_buf_internal()[index];
    }
    [[nodiscard]] wchar_t& operator[](size_t index) { return get_buf_internal()[index]; }
    [[nodiscard]] wchar_t operator[](size_t index) const { return get_buf_internal()[index]; }
    // various wchar_t checks

    // single wchar_t [last_]index[_not]_of functions
    [[nodiscard]] size_t index_of(wchar_t c, size_t start_index = 0) const {
        if (m_size == 0) return npos;
        if (start_index >= m_size) return npos;
        const wchar_t* buf = get_buf_internal();
        for (size_t i = start_index; i < m_size; i++) {
            if (buf[i] == c) return i;
        }
        return npos;
    }
    [[nodiscard]] size_t last_index_of(wchar_t c, size_t end_index = npos) const {
        if (m_size == 0) return npos;
        const wchar_t* buf = get_buf_internal();
        for (size_t i = (end_index > m_size - 1) ? m_size - 1 : end_index;; i--) {
            if (buf[i] == c) return i;
            if (i == 0) break;
        }
        if (buf[0] == c) return 0ull;
        return npos;
    }
    [[nodiscard]] size_t index_not_of(wchar_t c, size_t start_index = 0) const {
        if (m_size == 0) return npos;
        if (start_index >= m_size) return npos;
        const wchar_t* buf = get_buf_internal();
        for (size_t i = start_index; i < m_size; i++) {
            if (buf[i] != c) return i;
        }
        return npos;
    }
    [[nodiscard]] size_t last_index_not_of(wchar_t c, size_t end_index = npos) const {
        if (m_size == 0) return npos;
        const wchar_t* buf = get_buf_internal();
        for (size_t i = (end_index > m_size - 1) ? m_size - 1 : end_index;; i--) {
            if (buf[i] != c) return i;
            if (i == 0) break;
        }
        return npos;
    }
    // span [last_]index[_not]_of functions
    [[nodiscard]] size_t index_of(WStringView c, size_t start = 0) const;
    [[nodiscard]] size_t last_index_of(WStringView c, size_t end = npos) const;
    [[nodiscard]] size_t index_not_of(WStringView c, size_t start = 0) const;
    [[nodiscard]] size_t last_index_not_of(WStringView c, size_t end = npos) const;

    // any wchar_t in span
    [[nodiscard]] size_t index_of_any(WStringView any, size_t start_index = 0ull) const;
    [[nodiscard]] size_t last_index_of_any(WStringView any, size_t end_index = npos) const;
    [[nodiscard]] size_t index_not_of_any(WStringView any, size_t start_index = 0ull) const;
    [[nodiscard]] size_t last_index_not_of_any(WStringView any, size_t end_index = npos) const;

    [[nodiscard]] bool contains(WStringView other) const;
    [[nodiscard]] bool contains(wchar_t c) const { return index_of(c) != npos; }
    // indexes
    [[nodiscard]] Vector<size_t> all_indexes_of(WStringView c, size_t start_idx = 0) const;
    // trim
    void irtrim() {
        if (m_size == 0) return;
        if (!wisspace(m_data_buf[0])) return;
        if (is_local()) {
            size_t count = 0;
            while (wisspace(m_local_buf[count++]))
                ;
            count--;
            if (count != 0) {
                m_size -= count;
                ConditionalBitCopy(m_local_buf, m_local_buf + count, m_size);
            }
        } else {
            size_t count = 0;
            while (wisspace(m_data_buf[count++]))
                ;
            count--;
            if (count != 0) {
                m_size -= count;
                // if the m_size is now small enough, let's swap to small WString
                if (m_size <= SMALL_STRING_CAP) {
                    ConditionalBitCopy(m_local_buf, m_data_buf + count, m_size);
                    deallocate<wchar_t, DeallocType::Multiple>(m_data_buf);
                    m_data_buf = local_data_internal();
                } else {
                    ConditionalBitCopy(m_data_buf, m_data_buf + count, m_size);
                }
            }
        }
    }
    void iltrim() {
        if (m_size == 0) return;
        if (!wisspace(m_data_buf[m_size - 1])) return;
        if (is_local()) {
            while (wisspace(m_local_buf[m_size - 1]) && m_size > 0) m_size--;
            m_local_buf[m_size] = L'\0';
        } else {
            while (wisspace(m_data_buf[m_size - 1]) && m_size > 0) m_size--;
            m_data_buf[m_size] = L'\0';
            // if the m_size is now small enough, let's swap to small WString
            if (m_size <= SMALL_STRING_CAP) {
                ConditionalBitCopy(m_local_buf, m_data_buf, SMALL_STRING_CAP + 1);
                deallocate<wchar_t, DeallocType::Multiple>(m_data_buf);
                m_data_buf = local_data_internal();
            }
        }
    }
    void itrim() {
        irtrim();
        iltrim();
    }
    [[nodiscard]] WString ltrim() const {
        WString ret(*this);
        ret.iltrim();
        return ret;
    }
    [[nodiscard]] WString rtrim() const {
        WString ret(*this);
        ret.irtrim();
        return ret;
    }
    [[nodiscard]] WString trim() const {
        WString ret(*this);
        ret.itrim();
        return ret;
    }
    Vector<WString> split_at_any(const wchar_t* sep = L" \n\t\v") const;
    Vector<WStringView> split_view_at_any(const wchar_t* sep = L" \n\t\v") const;

    Vector<WString> split(const wchar_t* sep = L" ") const;
    Vector<WStringView> split_view(const wchar_t* sep = L" ") const;
    // upper/lower
    void iupper() {
        wchar_t* buf = get_buf_internal();
        for (size_t i = 0; i < m_size; i++) { buf[i] = wtoupper(buf[i]); }
    }
    void ilower() {
        wchar_t* buf = get_buf_internal();
        for (size_t i = 0; i < m_size; i++) { buf[i] = wtolower(buf[i]); }
    }
    [[nodiscard]] WString upper() const& {
        WString str(*this);
        str.iupper();
        return str;
    }
    [[nodiscard]] WString lower() const& {
        WString str(*this);
        str.ilower();
        return str;
    }
    [[nodiscard]] WString upper() && {
        WString str{ move(*this) };
        str.iupper();
        return str;
    }
    [[nodiscard]] WString lower() && {
        WString str{ move(*this) };
        str.ilower();
        return str;
    }
    // replace
    void ireplace(wchar_t n, wchar_t s, size_t times = WString::npos) {
        wchar_t* buf = get_buf_internal();
        for (size_t i = 0, j = 0; i < m_size && j < times; i++) {
            if (buf[i] == n) {
                buf[i] = s;
                j++;
            }
        }
    }
    WString replace(wchar_t n, wchar_t s, size_t times = WString::npos) {
        WString cp{ *this };
        cp.ireplace(n, s, times);
        return cp;
    }
    void ireplace(WStringView n, WStringView s, size_t times = WString::npos);
    WString replace(WStringView n, WStringView s, size_t times = WString::npos) const;
    void reserve(size_t new_capacity) { grow_if_needed(new_capacity); }
};
inline WString operator""_ws(const wchar_t* source, size_t len) {
    return WString{ source, len };
}
template <>
struct Hash<WString> {
    [[nodiscard]] size_t operator()(const WString& key) const noexcept {
        return hash_array_representation(key.data(), key.size());
    }
};
template <>
struct PrintInfo<WString> {
    const WString& m_wstr;
    PrintInfo(const WString& wstr) : m_wstr(wstr) {}
    String repr() const;
};
}    // namespace ARLib

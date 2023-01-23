#include "WString.h"
#include "Enumerate.h"
#include "Ordering.h"
#include "WStringView.h"
#include "Vector.h"
#include "String.h"
#include "StringView.h"
#include "arlib_osapi.h"
namespace ARLib {
WString::WString(const String& other) {
    *this = string_to_wstring(other.view());
}
WString::WString(StringView other) {
    *this = string_to_wstring(other);
}
[[nodiscard]] bool WString::operator==(const WStringView& other) const {
    auto thislen  = size();
    auto otherlen = other.size();
    auto buf      = get_buf_internal();
    if (thislen != otherlen) return false;
    for (size_t i = 0; i < thislen; i++) {
        if (buf[i] != other[i]) return false;
    }
    return true;
}
[[nodiscard]] bool WString::operator!=(const WStringView& other) const {
    return !(*this == other);
}
[[nodiscard]] bool WString::operator<(const WStringView& other) const {
    // we use the size() of the WStringView cause it's not guaranteed to be null terminated
    return wstrncmp(get_buf_internal(), other.data(), other.size());
}
[[nodiscard]] Ordering WString::operator<=>(const WString& other) const {
    auto val = wstrncmp(get_buf_internal(), other.get_buf_internal(), other.m_size);
    if (val == 0)
        return equal;
    else if (val < 0)
        return less;
    else
        return greater;
}
[[nodiscard]] Ordering WString::operator<=>(const WStringView& other) const {
    auto val = wstrncmp(get_buf_internal(), other.data(), other.size());
    if (val == 0)
        return equal;
    else if (val < 0)
        return less;
    else
        return greater;
}
[[nodiscard]] bool WString::starts_with(WStringView other) const {
    auto o_len = other.size();
    if (o_len > m_size) return false;
    if (m_size == o_len) return wstrcmp(other.data(), get_buf_internal()) == 0;
    auto res = wstrncmp(other.data(), get_buf_internal(), o_len);
    return res == 0;
}
[[nodiscard]] bool WString::ends_with(WStringView other) const {
    auto o_len = other.size();
    if (o_len > m_size) return false;
    if (m_size == o_len) return wstrcmp(other.data(), get_buf_internal()) == 0;
    auto ptrdiff          = m_size - o_len;
    const wchar_t* my_buf = get_buf_internal();
    auto res              = wstrncmp(my_buf + ptrdiff, other.data(), o_len);
    return res == 0;
}
[[nodiscard]] size_t WString::index_of(WStringView c, size_t start) const {
    if (m_size == 0 || start >= m_size) return npos;
    const wchar_t* buf = get_buf_internal();
    auto o_len         = c.size();
    if (o_len > m_size) return npos;
    if (start + o_len > m_size) return npos;
    if (o_len == m_size && start == 0 && wstrcmp(buf, c.data()) == 0) return 0;
    for (size_t i = start; i < m_size; i++) {
        if (wstrncmp(buf + i, c.data(), o_len) == 0) return i;
    }
    return npos;
}
[[nodiscard]] size_t WString::last_index_of(WStringView c, size_t end) const {
    if (m_size == 0) return npos;
    const wchar_t* buf = get_buf_internal();
    auto o_len         = c.size();
    if (end < o_len || o_len > m_size) return npos;
    if (end > m_size) end = m_size;
    if (o_len == end && wstrncmp(buf, c.data(), end) == 0) return 0;
    for (size_t i = end - o_len;; i--) {
        if (wstrncmp(buf + i, c.data(), o_len) == 0) return i;
        if (i == 0) break;
    }
    return npos;
}
[[nodiscard]] size_t WString::index_not_of(WStringView c, size_t start) const {
    if (m_size == 0 || start >= m_size) return npos;
    const wchar_t* buf = get_buf_internal();
    auto o_len         = c.size();
    if (start + o_len > m_size) return npos;
    if (o_len > m_size) return npos;
    if (o_len == m_size && start == 0 && wstrcmp(buf, c.data()) != 0) return 0;
    for (size_t i = start; i < m_size; i++) {
        if (wstrncmp(buf + i, c.data(), o_len) != 0) return i;
    }
    return npos;
}
[[nodiscard]] size_t WString::last_index_not_of(WStringView c, size_t end) const {
    if (m_size == 0) return npos;
    const wchar_t* buf = get_buf_internal();
    auto o_len         = c.size();
    if (end < o_len || o_len > m_size) return npos;
    if (end > m_size) end = m_size;
    if (o_len == end && wstrncmp(buf, c.data(), end) != 0) return 0;
    for (size_t i = end - o_len;; i--) {
        if (wstrncmp(buf + i, c.data(), o_len) != 0) return i;
        if (i == 0) break;
    }
    return npos;
}
[[nodiscard]] Vector<size_t> WString::all_indexes_of(WStringView c, size_t start_idx) const {
    return all_indexes_internal(c, start_idx);
}
[[nodiscard]] bool WString::contains(WStringView other) const {
    return index_of(other) != npos;
}
[[nodiscard]] WStringView WString::substringview(size_t first, size_t last) const {
    if (first >= m_size) return WStringView{ get_buf_internal(), static_cast<size_t>(0) };
    if (last == npos || last >= m_size) last = m_size;
    return WStringView{ get_buf_internal() + first, last - first };
}
[[nodiscard]] WStringView WString::view() {
    auto ptr = get_buf_internal();
    return WStringView{ ptr, ptr + m_size };
}
[[nodiscard]] WStringView WString::view() const {
    const auto ptr = get_buf_internal();
    return WStringView{ ptr, ptr + m_size };
}
WString::WString(WStringView other) : m_size(other.length()) {
    bool local = m_size <= SMALL_STRING_CAP;
    if (local) {
        ConditionalBitCopy(m_local_buf, other.data(), m_size);
        m_data_buf = local_data_internal();
    } else {
        m_data_buf = new wchar_t[m_size + 1];
        ConditionalBitCopy(m_data_buf, other.data(), m_size);
    }
}
Vector<WString> WString::split_at_any(const wchar_t* sep) const {
    WStringView sep_view{ sep };
    auto indexes = all_indexes_internal(sep_view);
    Vector<WString> vec{};
    vec.reserve(indexes.size() + 1);
    size_t prev_index = 0;
    for (auto index : indexes) {
        if (prev_index > index) prev_index = 0;
        vec.append(substring(prev_index, index));
        prev_index = index + 1;
    }
    vec.append(substring(prev_index));
    return vec;
}
Vector<WString> WString::split(const wchar_t* sep) const {
    Vector<WString> vec{};
    WStringView sep_view{ sep };
    size_t prev_index = 0ull;
    size_t index      = index_of(sep_view);
    while (index != npos) {
        vec.append(substring(prev_index, index));
        prev_index = index + sep_view.size();
        index      = index_of(sep_view, prev_index);
    }
    vec.append(substring(prev_index));
    return vec;
}
Vector<WStringView> WString::split_view_at_any(const wchar_t* sep) const {
    WStringView sep_view{ sep };
    auto indexes = all_indexes_internal(sep_view);
    Vector<WStringView> vec{};
    vec.reserve(indexes.size() + 1);
    size_t prev_index = 0;
    for (auto index : indexes) {
        vec.append(substringview(prev_index, index));
        prev_index = index + 1;
    }
    vec.append(substringview(prev_index));
    return vec;
}
Vector<WStringView> WString::split_view(const wchar_t* sep) const {
    WStringView sep_view{ sep };
    Vector<WStringView> vec{};
    size_t sep_len    = sep_view.size();
    size_t prev_index = 0ull;
    size_t index      = index_of(sep_view);
    while (index != npos) {
        vec.append(substringview(prev_index, index));
        prev_index = index + sep_len;
        index      = index_of(sep_view, prev_index);
    }
    vec.append(substringview(prev_index));
    return vec;
}
Vector<size_t> WString::all_indexes_internal(WStringView any, size_t start_index) const {
    auto size = any.size();
    Vector<size_t> indexes{};
    for (size_t i = 0; i < size; i++) {
        auto index = index_of(any[i], start_index);
        while (index != npos) {
            indexes.push_back(index);
            index = index_of(any[i], index + 1);
        }
    }
    return indexes;
}
Vector<size_t> WString::all_last_indexes_internal(WStringView any, size_t end_index) const {
    auto size = any.size();
    Vector<size_t> indexes{};
    for (size_t i = 0; i < size; i++) {
        auto index = last_index_of(any[i], end_index);
        while (index != npos && index != 0) {
            indexes.push_back(index);
            index = last_index_of(any[i], index - 1);
        }
    }
    return indexes;
}
Vector<size_t> WString::all_not_indexes_internal(WStringView any, size_t start_index) const {
    auto size = any.size();
    Vector<size_t> indexes{};
    for (size_t i = 0; i < size; i++) {
        auto index = index_not_of(any[i], start_index);
        while (index != npos) {
            indexes.push_back(index);
            index = index_not_of(any[i], index + 1);
        }
    }
    return indexes;
}
Vector<size_t> WString::all_last_not_indexes_internal(WStringView any, size_t end_index) const {
    auto size = any.size();
    Vector<size_t> indexes{};
    for (size_t i = 0; i < size; i++) {
        auto index = last_index_not_of(any[i], end_index);
        while (index != npos && index != 0) {
            indexes.push_back(index);
            index = last_index_not_of(any[i], index - 1);
        }
    }
    return indexes;
}
[[nodiscard]] size_t WString::index_of_any(WStringView any, size_t start_index) const {
    auto indexes = all_indexes_internal(any, start_index);
    if (indexes.size() == 0) return npos;
    return *min(indexes);
}
[[nodiscard]] size_t WString::last_index_of_any(WStringView any, size_t end_index) const {
    auto indexes = all_last_indexes_internal(any, end_index);
    if (indexes.size() == 0) return npos;
    return *max(indexes);
}
[[nodiscard]] size_t WString::index_not_of_any(WStringView any, size_t start_index) const {
    auto indexes = all_not_indexes_internal(any, start_index);
    if (indexes.size() == 0) return npos;
    return *min(indexes);
}
[[nodiscard]] size_t WString::last_index_not_of_any(WStringView any, size_t end_index) const {
    auto indexes = all_last_not_indexes_internal(any, end_index);
    if (indexes.size() == 0) return npos;
    return *max(indexes);
}
void WString::ireplace(WStringView n, WStringView s, size_t times) {
    size_t orig_len = n.size();
    if (orig_len > m_size) return;
    Vector<size_t> indexes{};
    size_t cur_pos = 0;
    wchar_t* buf   = get_buf_internal();
    while (cur_pos < m_size && indexes.size() <= times) {
        if (wstrncmp(buf + cur_pos, n.data(), orig_len) == 0) {
            indexes.push_back(cur_pos);
            cur_pos += orig_len;
        } else {
            cur_pos++;
        }
    }
    auto n_occurr = indexes.size();
    if (n_occurr == 0ull) return;
    size_t repl_len     = s.size();
    bool repl_is_bigger = repl_len > orig_len;
    size_t diff_len     = repl_is_bigger ? repl_len - orig_len : orig_len - repl_len;
    if (diff_len > 0) {
        reserve(m_size + n_occurr * diff_len);
        buf = get_buf_internal();
    }
    for (auto [count, index] : Enumerate{ indexes }) {
        auto new_index = repl_is_bigger ? index + (count * diff_len) : index - (count * diff_len);
        if (repl_is_bigger)
            ConditionalBitCopy(buf + new_index + diff_len, buf + new_index, m_size - index + 1ull);
        else if (diff_len != 0)
            ConditionalBitCopy(buf + new_index, buf + new_index + diff_len, m_size - index + 1ull);
        ConditionalBitCopy(buf + new_index, s.data(), repl_len);
    }
    set_size(repl_is_bigger ? m_size + diff_len * n_occurr : m_size - diff_len * n_occurr);
}
WString WString::replace(WStringView n, WStringView s, size_t times) const {
    WString str{ *this };
    str.ireplace(n, s, times);
    return str;
}
void WString::append(WStringView other) {
    auto other_size = other.size();
    if (other_size == 0) return;
    auto new_size = m_size + other_size;
    grow_if_needed(new_size);
    ConditionalBitCopy(get_buf_internal() + m_size, other.data(), other_size);
    set_size(new_size);
}
void WString::append(const wchar_t* other) {
    WStringView view{ other };
    append(view);
}
WString WString::concat(WStringView other) const {
    auto other_size = other.size();
    WString copy{ *this };
    if (other_size == 0) return copy;
    auto new_size = m_size + other_size;
    copy.grow_if_needed(new_size);
    ConditionalBitCopy(copy.get_buf_internal() + m_size, other.data(), other_size);
    copy.set_size(new_size);
    return copy;
}
WString WString::concat(const wchar_t* other) const {
    WString copy{ *this };
    WStringView sother{ other };
    copy.append(sother);
    return copy;
}
String PrintInfo<WString>::repr() const {
    return wstring_to_string(m_wstr.view());
}
}    // namespace ARLib

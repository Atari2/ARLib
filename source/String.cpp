#include "String.hpp"
#include "Enumerate.hpp"
#include "Ordering.hpp"
#include "StringView.hpp"
#include "Vector.hpp"
namespace ARLib {
[[nodiscard]] bool String::operator==(const StringView& other) const {
    auto thislen  = size();
    auto otherlen = other.size();
    auto buf      = get_buf_internal();
    if (thislen != otherlen) return false;
    for (size_t i = 0; i < thislen; i++) {
        if (buf[i] != other[i]) return false;
    }
    return true;
}
[[nodiscard]] bool String::operator<(const StringView& other) const {
    // we use the size() of the stringview cause it's not guaranteed to be null terminated
    return strncmp(get_buf_internal(), other.data(), other.size());
}
[[nodiscard]] Ordering String::operator<=>(const String& other) const {
    auto val = strncmp(get_buf_internal(), other.get_buf_internal(), other.m_size);
    if (val == 0)
        return equal;
    else if (val < 0)
        return less;
    else
        return greater;
}
[[nodiscard]] Ordering String::operator<=>(const StringView& other) const {
    auto val = strncmp(get_buf_internal(), other.data(), other.size());
    if (val == 0)
        return equal;
    else if (val < 0)
        return less;
    else
        return greater;
}
[[nodiscard]] bool String::starts_with(StringView other) const {
    auto o_len = other.size();
    if (o_len > m_size) return false;
    if (m_size == o_len) return strcmp(other.data(), get_buf_internal()) == 0;
    auto res = strncmp(other.data(), get_buf_internal(), o_len);
    return res == 0;
}
[[nodiscard]] bool String::ends_with(StringView other) const {
    auto o_len = other.size();
    if (o_len > m_size) return false;
    if (m_size == o_len) return strcmp(other.data(), get_buf_internal()) == 0;
    auto ptrdiff       = m_size - o_len;
    const char* my_buf = get_buf_internal();
    auto res           = strncmp(my_buf + ptrdiff, other.data(), o_len);
    return res == 0;
}
[[nodiscard]] size_t String::index_of(StringView c, size_t start) const {
    if (m_size == 0 || start >= m_size) return npos;
    const char* buf = get_buf_internal();
    auto o_len      = c.size();
    if (o_len > m_size) return npos;
    if (start + o_len > m_size) return npos;
    if (o_len == m_size && start == 0 && strcmp(buf, c.data()) == 0) return 0;
    for (size_t i = start; i < m_size; i++) {
        if (strncmp(buf + i, c.data(), o_len) == 0) return i;
    }
    return npos;
}
[[nodiscard]] size_t String::last_index_of(StringView c, size_t end) const {
    if (m_size == 0) return npos;
    const char* buf = get_buf_internal();
    auto o_len      = c.size();
    if (end < o_len || o_len > m_size) return npos;
    if (end > m_size) end = m_size;
    if (o_len == end && strncmp(buf, c.data(), end) == 0) return 0;
    for (size_t i = end - o_len;; i--) {
        if (strncmp(buf + i, c.data(), o_len) == 0) return i;
        if (i == 0) break;
    }
    return npos;
}
[[nodiscard]] size_t String::index_not_of(StringView c, size_t start) const {
    if (m_size == 0 || start >= m_size) return npos;
    const char* buf = get_buf_internal();
    auto o_len      = c.size();
    if (start + o_len > m_size) return npos;
    if (o_len > m_size) return npos;
    if (o_len == m_size && start == 0 && strcmp(buf, c.data()) != 0) return 0;
    for (size_t i = start; i < m_size; i++) {
        if (strncmp(buf + i, c.data(), o_len) != 0) return i;
    }
    return npos;
}
[[nodiscard]] size_t String::last_index_not_of(StringView c, size_t end) const {
    if (m_size == 0) return npos;
    const char* buf = get_buf_internal();
    auto o_len      = c.size();
    if (end < o_len || o_len > m_size) return npos;
    if (end > m_size) end = m_size;
    if (o_len == end && strncmp(buf, c.data(), end) != 0) return 0;
    for (size_t i = end - o_len;; i--) {
        if (strncmp(buf + i, c.data(), o_len) != 0) return i;
        if (i == 0) break;
    }
    return npos;
}
[[nodiscard]] Vector<size_t> String::all_indexes_of(StringView c, size_t start_idx) const {
    return all_indexes_internal(c, start_idx);
}
[[nodiscard]] bool String::contains(StringView other) const {
    return index_of(other) != npos;
}
[[nodiscard]] StringView String::substringview(size_t first, size_t last) const {
    if (first >= m_size) return StringView{ get_buf_internal(), static_cast<size_t>(0) };
    if (last == npos || last >= m_size) last = m_size;
    return StringView{ get_buf_internal() + first, last - first };
}
[[nodiscard]] StringView String::view() {
    auto ptr = get_buf_internal();
    return StringView{ ptr, ptr + m_size };
}
[[nodiscard]] StringView String::view() const {
    const auto ptr = get_buf_internal();
    return StringView{ ptr, ptr + m_size };
}
String::String(StringView other) : m_size(other.length()) {
    bool local = m_size <= SMALL_STRING_CAP;
    if (local) {
        strncpy(m_local_buf, other.data(), m_size);
        m_data_buf = local_data_internal();
    } else {
        m_data_buf = allocate_uninitialized<char>(m_size + 1);
        strncpy(m_data_buf, other.data(), m_size);
        m_data_buf[m_size] = '\0';
    }
}
Vector<String> String::split_at_any(const char* sep) const {
    StringView sep_view{ sep };
    auto indexes = all_indexes_internal(sep_view);
    Vector<String> vec{};
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
Vector<String> String::split(const char* sep) const {
    Vector<String> vec{};
    StringView sep_view{ sep };
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
Vector<StringView> String::split_view_at_any(const char* sep) const {
    StringView sep_view{ sep };
    auto indexes = all_indexes_internal(sep_view);
    Vector<StringView> vec{};
    vec.reserve(indexes.size() + 1);
    size_t prev_index = 0;
    for (auto index : indexes) {
        vec.append(substringview(prev_index, index));
        prev_index = index + 1;
    }
    vec.append(substringview(prev_index));
    return vec;
}
Vector<StringView> String::split_view(const char* sep) const {
    StringView sep_view{ sep };
    Vector<StringView> vec{};
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
Vector<size_t> String::all_indexes_internal(StringView any, size_t start_index) const {
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
Vector<size_t> String::all_last_indexes_internal(StringView any, size_t end_index) const {
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
Vector<size_t> String::all_not_indexes_internal(StringView any, size_t start_index) const {
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
Vector<size_t> String::all_last_not_indexes_internal(StringView any, size_t end_index) const {
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
[[nodiscard]] size_t String::index_of_any(StringView any, size_t start_index) const {
    auto indexes = all_indexes_internal(any, start_index);
    if (indexes.size() == 0) return npos;
    return *min(indexes);
}
[[nodiscard]] size_t String::last_index_of_any(StringView any, size_t end_index) const {
    auto indexes = all_last_indexes_internal(any, end_index);
    if (indexes.size() == 0) return npos;
    return *max(indexes);
}
[[nodiscard]] size_t String::index_not_of_any(StringView any, size_t start_index) const {
    auto indexes = all_not_indexes_internal(any, start_index);
    if (indexes.size() == 0) return npos;
    return *min(indexes);
}
[[nodiscard]] size_t String::last_index_not_of_any(StringView any, size_t end_index) const {
    auto indexes = all_last_not_indexes_internal(any, end_index);
    if (indexes.size() == 0) return npos;
    return *max(indexes);
}
void String::ireplace(StringView n, StringView s, size_t times) {
    size_t orig_len = n.size();
    if (orig_len > m_size) return;
    Vector<size_t> indexes{};
    size_t cur_pos = 0;
    char* buf      = get_buf_internal();
    while (cur_pos < m_size && indexes.size() <= times) {
        if (strncmp(buf + cur_pos, n.data(), orig_len) == 0) {
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
            memmove(buf + new_index + diff_len, buf + new_index, m_size - index + 1ull);
        else if (diff_len != 0)
            memmove(buf + new_index, buf + new_index + diff_len, m_size - index + 1ull);
        memcpy(buf + new_index, s.data(), repl_len);
    }
    set_size(repl_is_bigger ? m_size + diff_len * n_occurr : m_size - diff_len * n_occurr);
}
String String::replace(StringView n, StringView s, size_t times) const {
    String str{ *this };
    str.ireplace(n, s, times);
    return str;
}
void String::append(StringView other) {
    auto other_size = other.size();
    if (other_size == 0) return;
    auto new_size = m_size + other_size;
    grow_if_needed(new_size);
    memcpy(get_buf_internal() + m_size, other.data(), other_size);
    set_size(new_size);
}
void String::append(const char* other) {
    StringView view{ other };
    append(view);
}
String String::concat(StringView other) const& {
    auto other_size = other.size();
    String copy{ *this };
    if (other_size == 0) return copy;
    auto new_size = m_size + other_size;
    copy.grow_if_needed(new_size);
    memcpy(copy.get_buf_internal() + m_size, other.data(), other_size);
    copy.set_size(new_size);
    return copy;
}
String String::concat(const char* other) const& {
    String copy{ *this };
    StringView sother{ other };
    copy.append(sother);
    return copy;
}
String String::concat(StringView other)&& {
    auto other_size = other.size();
    String moved{ move(*this) };
    if (other_size == 0) return moved;
    auto new_size = moved.m_size + other_size;
    moved.grow_if_needed(new_size);
    memcpy(moved.get_buf_internal() + moved.m_size, other.data(), other_size);
    moved.set_size(new_size);
    return moved;
}
String String::concat(const char* other)&& {
    String moved{ move(*this) };
    StringView sother{ other };
    moved.append(sother);
    return moved;
}
}    // namespace ARLib

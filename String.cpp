#include "String.h"
#include "Enumerate.h"
#include "Ordering.h"
#include "StringView.h"
#include "Vector.h"

namespace ARLib {
    [[nodiscard]] bool String::operator==(const StringView& other) const {
        auto thislen = size();
        auto otherlen = other.size();
        auto buf = get_buf_internal();
        if (thislen != otherlen) return false;
        for (size_t i = 0; i < thislen; i++) {
            if (buf[i] != other[i]) return false;
        }
        return true;
    }

    [[nodiscard]] bool String::operator!=(const StringView& other) const { return !(*this == other); }

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

    [[nodiscard]] StringView String::substringview(size_t first, size_t last) const {
        if (first >= m_size) return StringView{get_buf_internal(), static_cast<size_t>(0)};
        if (last == npos || last >= m_size) last = m_size;
        return StringView{get_buf_internal() + first, last - first};
    }

    [[nodiscard]] StringView String::view() {
        auto ptr = get_buf_internal();
        return StringView{ptr, ptr + m_size};
    }

    String::String(StringView other) : m_size(other.length()) {
        bool local = m_size <= SMALL_STRING_CAP;
        if (local) {
            strcpy(m_local_buf, other.data());
            m_data_buf = local_data_internal();
        } else {
            m_data_buf = new char[m_size + 1];
            strcpy(m_data_buf, other.data());
        }
    }

    Vector<String> String::split_at_any(const char* sep) const {
        auto indexes = all_indexes_internal(sep);
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
        size_t sep_len = strlen(sep);
        size_t prev_index = 0ull;
        size_t index = index_of(sep);
        while (index != npos) {
            vec.append(substring(prev_index, index));
            prev_index = index + sep_len;
            index = index_of(sep, prev_index);
        }
        vec.append(substring(prev_index));
        return vec;
    }

    Vector<StringView> String::split_view_at_any(const char* sep) const {
        auto indexes = all_indexes_internal(sep);
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
        Vector<StringView> vec{};
        size_t sep_len = strlen(sep);
        size_t prev_index = 0ull;
        size_t index = index_of(sep);
        while (index != npos) {
            vec.append(substringview(prev_index, index));
            prev_index = index + sep_len;
            index = index_of(sep, prev_index);
        }
        vec.append(substringview(prev_index));
        return vec;
    }

    Vector<size_t> String::all_indexes_internal(const char* any, size_t start_index) const {
        auto size = strlen(any);
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
    Vector<size_t> String::all_last_indexes_internal(const char* any, size_t end_index) const {
        auto size = strlen(any);
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
    Vector<size_t> String::all_not_indexes_internal(const char* any, size_t start_index) const {
        auto size = strlen(any);
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
    Vector<size_t> String::all_last_not_indexes_internal(const char* any, size_t end_index) const {
        auto size = strlen(any);
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

    [[nodiscard]] size_t String::index_of_any(const char* any, size_t start_index) const {
        auto indexes = all_indexes_internal(any, start_index);
        return *min(indexes);
    }
    [[nodiscard]] size_t String::last_index_of_any(const char* any, size_t end_index) const {
        auto indexes = all_last_indexes_internal(any, end_index);
        return *max(indexes);
    }
    [[nodiscard]] size_t String::index_not_of_any(const char* any, size_t start_index) const {
        auto indexes = all_not_indexes_internal(any, start_index);
        return *min(indexes);
    }
    [[nodiscard]] size_t String::last_index_not_of_any(const char* any, size_t end_index) const {
        auto indexes = all_last_not_indexes_internal(any, end_index);
        return *max(indexes);
    }

    void String::ireplace(const char* n, const char* s, size_t times) {
        size_t orig_len = strlen(n);
        if (orig_len > m_size) return;
        Vector<size_t> indexes{};
        size_t cur_pos = 0;
        char* buf = get_buf_internal();
        while (cur_pos < m_size && indexes.size() <= times) {
            if (strncmp(buf + cur_pos, n, orig_len) == 0) {
                indexes.push_back(cur_pos);
                cur_pos += orig_len;
            } else {
                cur_pos++;
            }
        }
        auto n_occurr = indexes.size();
        if (n_occurr == 0ull) return;
        size_t repl_len = strlen(s);
        bool repl_is_bigger = repl_len > orig_len;
        size_t diff_len = repl_is_bigger ? repl_len - orig_len : orig_len - repl_len;
        if (diff_len > 0) {
            reserve(m_size + n_occurr * diff_len);
            buf = get_buf_internal();
        }
        for (auto [count, index] : Enumerate{indexes}) {
            auto new_index = repl_is_bigger ? index + (count * diff_len) : index - (count * diff_len);
            if (repl_is_bigger)
                memmove(buf + new_index + diff_len, buf + new_index, m_size - index + 1ull);
            else if (diff_len != 0)
                memmove(buf + new_index, buf + new_index + diff_len, m_size - index + 1ull);
            memcpy(buf + new_index, s, repl_len);
        }
        set_size(repl_is_bigger ? m_size + diff_len * n_occurr : m_size - diff_len * n_occurr);
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
        StringView view{other};
        append(view);
    }

    String String::concat(StringView other) const {
        auto other_size = other.size();
        String copy{*this};
        if (other_size == 0) return copy;
        auto new_size = m_size + other_size;
        copy.grow_if_needed(new_size);
        memcpy(copy.get_buf_internal() + m_size, other.data(), other_size);
        copy.set_size(new_size);
        return copy;
    }

    String String::concat(const char* other) const {
        String copy{*this};
        StringView sother{other};
        copy.append(sother);
        return copy;
    }

} // namespace ARLib

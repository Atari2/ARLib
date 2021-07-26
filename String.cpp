#include "String.h"
#include "StringView.h"

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

    [[nodiscard]] bool String::operator!=(const StringView& other) const { return !(*this == other); };

    [[nodiscard]] bool String::operator<(const StringView& other) const {
        // we use the size() of the stringview cause it's not guaranteed to be null terminated
        return strncmp(get_buf_internal(), other.data(), other.size());
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
        if (first >= m_size) return StringView{get_buf_internal(), 0ull};
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

    Vector<StringView> String::split_view(const char* sep) const {
        auto indexes = all_indexes(sep);
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

    void String::concat(StringView other) {
        auto other_size = other.size();
        if (other_size == 0) return;
        auto new_size = m_size + other_size;
        grow_if_needed(new_size);
        memcpy(get_buf_internal() + m_size, other.data(), other_size);
        set_size(new_size);
    }

    String operator""_s(const char* source, size_t len) { return String{source, len}; }
} // namespace ARLib

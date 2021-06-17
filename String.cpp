#include "String.h"
#include "StringView.h"

namespace ARLib {
    [[nodiscard]] StringView String::substringview(size_t first, size_t last) {
        if (last == npos)
            last = length();
        return StringView{ get_buf_internal() + first, last - first };
    }

    [[nodiscard]] StringView String::view() {
        auto ptr = get_buf_internal();
        return StringView{ ptr, ptr + m_size };
    }

    String::String(StringView other) : m_size(other.length()) {
        bool local = m_size <= SMALL_STRING_CAP;
        if (local) {
            strcpy(m_local_buf, other.data());
            m_data_buf = local_data_internal();
        }
        else {
            m_data_buf = new char[m_size + 1];
            strcpy(m_data_buf, other.data());
        }
    }

    Vector<StringView> String::split_view(const char* sep) {
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

    String operator""_s(const char* source, size_t len) {
        return String{ source, len + 1 };
    }
}

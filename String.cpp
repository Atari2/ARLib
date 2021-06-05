#include "StringView.h"

namespace ARLib {
    [[nodiscard]] StringView String::substringview(size_t first, size_t last) {
        if (last == npos)
            last = length();
        return StringView{ get_buf_internal() + first, last - first };
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

}

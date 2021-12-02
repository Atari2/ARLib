#include "StringView.h"
#include "Ordering.h"
#include "Vector.h"

namespace ARLib {
    [[nodiscard]] Ordering StringView::operator<=>(const StringView& other) const {
        auto val = strncmp(m_start, other.m_start, other.size());
        if (val == 0)
            return equal;
        else if (val < 0)
            return less;
        else
            return greater;
    }

    [[nodiscard]] size_t StringView::index_of(const char* c, size_t start) const {
        if (m_size == 0 || start >= m_size) return npos;
        const char* buf = m_start;
        auto o_len = strlen(c);
        if (o_len > m_size) return npos;
        if (start + o_len > m_size) return npos;
        if (o_len == m_size && start == 0 && strcmp(buf, c) == 0) return 0;
        for (size_t i = start; i < m_size; i++) {
            if (strncmp(buf + i, c, o_len) == 0) return i;
        }
        return npos;
    }

    Vector<StringView> StringView::split(const char* sep) const {
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
} // namespace ARLib
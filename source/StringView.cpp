#include "StringView.hpp"
#include "Ordering.hpp"
#include "Vector.hpp"
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
Vector<StringView> StringView::split(const char* sep) const {
    Vector<StringView> vec{};
    size_t sep_len    = strlen(sep);
    size_t prev_index = 0ull;
    size_t index      = index_of(sep);
    while (index != npos) {
        vec.append(substringview(prev_index, index));
        prev_index = index + sep_len;
        index      = index_of(sep, prev_index);
    }
    vec.append(substringview(prev_index));
    return vec;
}
}    // namespace ARLib

#include "WStringView.h"
#include "Ordering.h"
#include "Vector.h"
#include "arlib_osapi.h"
namespace ARLib {
[[nodiscard]] Ordering WStringView::operator<=>(const WStringView& other) const {
    auto val = wstrncmp(m_start, other.m_start, other.size());
    if (val == 0)
        return equal;
    else if (val < 0)
        return less;
    else
        return greater;
}
Vector<WStringView> WStringView::split(const wchar_t* sep) const {
    Vector<WStringView> vec{};
    size_t sep_len    = wstrlen(sep);
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
String PrintInfo<WStringView>::repr() const {
    return wstring_to_string(m_view);
}
}    // namespace ARLib

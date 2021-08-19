#include "StringView.h"
#include "Ordering.h"

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
} // namespace ARLib
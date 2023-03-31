#include <compare>
#include "Ordering.h"
namespace ARLib {
Ordering::Ordering(const std::strong_ordering& ord) {
    if (ord == std::strong_ordering::equal || ord == std::strong_ordering::equivalent)
        m_type = OrderingType::Equal;
    else if (ord == std::strong_ordering::greater)
        m_type = OrderingType::Greater;
    else
        m_type = OrderingType::Less;
}
Ordering::Ordering(const std::partial_ordering& ord) {
    if (ord == std::partial_ordering::equivalent)
        m_type = OrderingType::Equal;
    else if (ord == std::partial_ordering::greater)
        m_type = OrderingType::Greater;
    else if (ord == std::partial_ordering::less)
        m_type = OrderingType::Less;
    else
        m_type = OrderingType::NoOrder;
}
}    // namespace ARLib

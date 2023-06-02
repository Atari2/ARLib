#include "Ordering.hpp"
#include "PrintInfo.hpp"
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
template <>
struct PrintInfo<Ordering> {
    const Ordering& m_ordering;
    PrintInfo(const Ordering& ordering) : m_ordering(ordering) {}
    String repr() const {
        switch (m_ordering.type()) {
            case OrderingType::Less:
                return "less"_s;
            case OrderingType::Equal:
                return "equal"_s;
            case OrderingType::Greater:
                return "greater"_s;
            case OrderingType::NoOrder:
                return "unordered"_s;
        }
        arlib_unreachable;
    }
};
}    // namespace ARLib

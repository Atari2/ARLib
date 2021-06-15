#include <compare>
#include "Ordering.h"

namespace ARLib {
	Ordering::Ordering(const std::strong_ordering& ord) {
		if (ord == std::strong_ordering::equal)
			m_type = OrderingType::Equal;
		else if (ord == std::strong_ordering::greater)
			m_type = OrderingType::Greater;
		else
			m_type = OrderingType::Less;
	}
}
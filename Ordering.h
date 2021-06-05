#pragma once

namespace ARLib {
	enum class OrderingType {
		Less,
		Equal,
		Greater
	};
	class Ordering {
		OrderingType m_type;
	public:
		Ordering(OrderingType type) : m_type(type) {}
		Ordering() = delete;
		Ordering(const Ordering&) = default;
		Ordering(Ordering&&) = default;
		Ordering& operator=(const Ordering&) = default;
		Ordering& operator=(Ordering&&) = default;
		constexpr OrderingType type() const { return m_type; }

		friend constexpr bool operator==(const Ordering& v, const Ordering& w) noexcept {
			return v.type() == w.type();
		}
	};

	static const inline Ordering less{ OrderingType::Less };
	static const inline Ordering equal{ OrderingType::Equal };
	static const inline Ordering greater{ OrderingType::Greater };

}

using ARLib::Ordering;
using ARLib::less;
using ARLib::equal;
using ARLib::greater;

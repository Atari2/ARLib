#pragma once
#include "String.h"
#include "Concepts.h"
#include "Assertion.h"
#include "cmath_compat.h"

namespace ARLib {

	enum class SupportedBase {
		Decimal,
		Hexadecimal,
		Binary,
		Octal
	};

	double StrToDouble(const String& str) {
		TODO(StrToDouble)
		return 0.0;
	}
	float StrToFloat(const String& str) {
		TODO(StrToFloat)
		return 0.0f;
	}
	int StrToInt(const String& str, int base = 10) {

		if (base < 2 || base > 36)
			return 0;

		size_t cur_index = 0;
		size_t max_index = str.length();
		// skip whitespace
		while (isspace(str[cur_index])) {
			cur_index++;
		}

		if (cur_index == max_index)
			return 0;

		int sign = 1;
		if (str[cur_index] == '+' || str[cur_index] == '-')
			sign = str[cur_index] == '+' ? 1 : -1;

		if (cur_index == max_index)
			return 0 * sign;

		if (base == 16) {
			if (str[cur_index] == '0' && tolower(str[cur_index + 1]) == 'x')
				cur_index += 2;
		}
		else if (base == 2) {
			if (str[cur_index] == '0' && tolower(str[cur_index + 1]) == 'b')
				cur_index += 2;
		}

		// skip leading zeros
		while (str[cur_index] == '0') {
			cur_index++;
		}

		if (cur_index == max_index)
			return 0 * sign;


		// 0-9 => 48-57
		// A-Z => 65-90

		int total = 0;
		double pw = 0.0;
		for (size_t opp = max_index - 1; opp >= cur_index; opp--) {
			char c = toupper(str[opp]);
			if (!isalnum(c))
				return total;
			int num = c >= 'A' ? (c - 55) : (c - 48);
			if (num >= base)
				return total;
			total += round((num * pow(static_cast<double>(base), pw)));
			pw += 1.0;
			if (opp == cur_index)
				break;
		}
		return total;
	}

	String DoubleToStr(double value) {
		TODO(DoubleToStr)
		return "";
	}
	String FloatToStr(float value) {
		TODO(FloatToStr)
		return "";
	}
	String IntToStr(int value, SupportedBase base = SupportedBase::Decimal) {
		String rev{};
		if (base == SupportedBase::Decimal) {
			int div = 10;
			if (value == 0)
				rev.append('0');
			while (value > 0) {
				int rem = fmod(static_cast<double>(value), div);
				rev.append((rem / (div / 10)) + 48);
				value -= rem;
				if (value > 0)
					div *= 10;
			}
		}
		else if (base == SupportedBase::Hexadecimal) {
			if (value == 0)
				rev.append('0');
			while (value > 0) {
				int rem = value % 16;
				if (rem > 9)
					rev.append(rem + 55);
				else
					rev.append(rem + 48);
				value >>= 4;
			}
			rev.append('x');
			rev.append('0');
		}
		else if (base == SupportedBase::Binary) {
			if (value == 0)
				rev.append('0');
			while (value > 0) {
				int rem = value % 2;
				rev.append(rem + 48);
				value >>= 1;
			}
			rev.append('b');
			rev.append('0');
		}
		return rev.reversed();
	}

	template <Stringable T>
	String ToString(const T& value) {
		if constexpr (IsSameV<T, String>)
			return value;
		return value.to_string();
	}
}

using ARLib::StrToInt;
using ARLib::IntToStr;
using ARLib::SupportedBase;
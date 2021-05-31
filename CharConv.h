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
		TODO(IntToStr)
		return "";
	}

	template <Stringable T>
	String ToString(const T& value) {
		return value.to_string();
	}
}

using ARLib::StrToInt;
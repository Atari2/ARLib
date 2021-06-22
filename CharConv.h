#pragma once
#include "String.h"
#include "StringView.h"
#include "Concepts.h"
#include "Assertion.h"
#include "cmath_compat.h"
#include "cstdio_compat.h"

namespace ARLib {

	enum class SupportedBase {
		Decimal,
		Hexadecimal,
		Binary,
		Octal
	};

	int StrViewToInt(StringView view, int base = 10) {
		if (base < 2 || base > 36)
			return 0;

		size_t cur_index = 0;
		size_t max_index = view.length();
		// skip whitespace
		while (isspace(view[cur_index])) {
			cur_index++;
		}

		if (cur_index == max_index)
			return 0;

		int sign = 1;
		if (view[cur_index] == '+' || view[cur_index] == '-')
			sign = view[cur_index] == '+' ? 1 : -1;

		if (cur_index == max_index)
			return 0 * sign;

		if (base == 16) {
			if (view[cur_index] == '0' && tolower(view[cur_index + 1]) == 'x')
				cur_index += 2;
		}
		else if (base == 2) {
			if (view[cur_index] == '0' && tolower(view[cur_index + 1]) == 'b')
				cur_index += 2;
		}

		// skip leading zeros
		while (view[cur_index] == '0') {
			cur_index++;
		}

		if (cur_index == max_index)
			return 0 * sign;


		// 0-9 => 48-57
		// A-Z => 65-90

		int total = 0;
		double pw = 0.0;
		for (size_t opp = max_index - 1; opp >= cur_index; opp--) {
			char c = toupper(view[opp]);
			if (!isalnum(c))
				return total;
			int num = c >= 'A' ? (c - 55) : (c - 48);
			if (num >= base)
				return total;
			total += static_cast<int>(round((num * pow(static_cast<double>(base), pw))));
			pw += 1.0;
			if (opp == cur_index)
				break;
		}
		return total;
	}

	int64_t StrToI64(const String& str, int base = 10) {
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
		char s = str[cur_index];
		if (s == '+' || s == '-') {
			sign = s == '+' ? 1 : -1;
			cur_index++;
		}

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

		int64_t total = 0;
		double pw = 0.0;
		for (size_t opp = max_index - 1; opp >= cur_index; opp--) {
			char c = toupper(str[opp]);
			if (!isalnum(c))
				return total;
			int num = c >= 'A' ? (c - 55) : (c - 48);
			if (num >= base)
				return total;
			total += static_cast<int64_t>(round((num * pow(static_cast<double>(base), pw))));
			pw += 1.0;
			if (opp == cur_index)
				break;
		}
		return total * sign;
	}

	int StrToInt(const String& str, int base = 10) {
		return static_cast<int>(StrToI64(str, base));
	}

	double StrToDouble(const String& str) {
		TODO(StrToDouble)
	}

	float StrToFloat(const String& str) {
		TODO(StrToFloat)
	}

	String DoubleToStr(double value) {
#ifdef WINDOWS
		const auto len = static_cast<size_t>(scprintf("%f", value));
		String str(len);
		sprintf(str.rawptr(), "%f", value);
		return str;
#else
		const int n = 308 /* numeric limits length for dbl */ + 20;
		String str(n);
		snprintf(str.rawptr(), n, "%f", value);
		return str;
#endif
	}
	String FloatToStr(float value) {
#ifdef WINDOWS
		return DoubleToStr(static_cast<double>(value));
#else
		const int n = 38 /* numeric limits length for flt */ + 20;
		String str(n);
		snprintf(str.rawptr(), n, "%f", value);
		return str;
#endif
	}


	template <class UnsignedIntegral>
	char* unsigned_to_buffer(char* next, UnsignedIntegral uvalue) {
#ifdef ENVIRON64
		auto uvalue_trunc = uvalue;
#else
		constexpr bool huge_unsigned = sizeof(UnsignedIntegral) > 4;
		if constexpr (huge_unsigned) {
			while (uvalue > 0xFFFFFFFFU) {
				auto uvalue_chunk = static_cast<unsigned long>(uvalue % 1000000000);
				uvalue /= 1000000000;
				for (int i = 0; i != 9; ++i) {
					*--next = static_cast<char>('0' + uvalue_chunk % 10);
					uvalue_chunk /= 10;
				}
			}
		}
		auto uvalue_trunc = static_cast<unsigned long>(uvalue);
#endif
		do {
			*--next = static_cast<char>('0' + uvalue_trunc % 10);
			uvalue_trunc /= 10;
		} while (uvalue_trunc != 0);
		return next;
	}


	template<class Integral, SupportedBase Base = SupportedBase::Decimal>
	String IntToStr(Integral value) {
		static_assert(IsIntegralV<Integral>, "IntToStr type must be integral type");
		if constexpr (Base == SupportedBase::Decimal) {
			char buf[22] = { 0 };
			char* const buf_end = end(buf) - 1;
			char* next = buf_end;
			const auto uvalue = static_cast<MakeUnsignedT<Integral>>(value);
			if (value < 0) {
				next = unsigned_to_buffer(next, 0 - uvalue);
				*--next = '-';
			}
			else {
				next = unsigned_to_buffer(next, uvalue);
			}
			return String{ next, buf_end };
		}
		else {
			String rev{};
			if constexpr (Base == SupportedBase::Hexadecimal) {
				if (value == 0)
					rev.append('0');
				while (value > 0) {
					Integral rem = value % 16;
					if (rem > 9)
						rev.append(rem + 55);
					else
						rev.append(rem + 48);
					value >>= 4;
				}
				rev.append('x');
				rev.append('0');
			}
			else if constexpr (Base == SupportedBase::Binary) {
				if (value == 0)
					rev.append('0');
				while (value > 0) {
					Integral rem = value % 2;
					rev.append(rem + 48);
					value >>= 1;
				}
				rev.append('b');
				rev.append('0');
			}
			return rev.reversed();
		}
	}

	bool StrToBool(const String& value) {
		if (value == "true")
			return true;
		else
			return false;
	}

	String BoolToStr(bool value) {
		if (value)
			return String{ "true" };
		return String{ "false" };
	}

	String ToString(Stringable auto& value) {
		if constexpr (IsSameV<decltype(value), String>)
			return value;
		return value.to_string();
	}
}

using ARLib::FloatToStr;
using ARLib::DoubleToStr;
using ARLib::StrToInt;
using ARLib::IntToStr;
using ARLib::StrToFloat;
using ARLib::SupportedBase;
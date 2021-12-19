#pragma once
#include "CharConvHelpers.h"

namespace ARLib {

    enum class SupportedBase { Decimal, Hexadecimal, Binary, Octal };

    // FIXME: make this more efficient
    double StrToDouble(const String& str);
    float StrToFloat(const String& str);
    String DoubleToStr(double value);
    String LongDoubleToStr(long double value);
    String FloatToStr(float value);

    // TODO: Add string_to_unsigned variants

    constexpr int64_t StrViewToI64(const StringView view, int base = 10) {
        if (base < 2 || base > 36) return 0;

        size_t cur_index = 0;
        size_t max_index = view.length();

        // skip leading and trailing whitespace
        if (max_index == 0) return 0;

        while (isspace(view[cur_index])) {
            cur_index++;
            if (cur_index == max_index) return 0;
        }

        while (isspace(view[max_index - 1])) {
            max_index--;
            if (max_index == npos_) return 0;
        }

        if (cur_index == max_index) return 0;

        switch (base) {
        case 2:
            return StrViewToI64Binary(view.substringview(cur_index, max_index));
        case 8:
            return StrViewToI64Octal(view.substringview(cur_index, max_index));
        case 10:
            return StrViewToI64Decimal(view.substringview(cur_index, max_index));
        case 16:
            return StrViewToI64Hexadecimal(view.substringview(cur_index, max_index));
        default:
            // bases other than 2, 8, 10 and 16 take the slow path
            break;
        }

        int sign = 1;
        char s = view[cur_index];
        if (s == '+' || s == '-') {
            sign = s == '+' ? 1 : -1;
            cur_index++;
        }

        if (cur_index == max_index) return 0ll * sign;

        // skip leading zeros
        while (view[cur_index] == '0') {
            cur_index++;
        }

        if (cur_index == max_index) return 0ll * sign;

        // 0-9 => 48-57
        // A-Z => 65-90

        // base is a power of 2, we can use bitshifts, else, use pow
        // allowed power of 2s are {4, 32}, because base is bounded [2, 36]
        // and 2, 8 and 16 are already handled elsewhere
        int64_t total = 0;
        if (base == 4 || base == 32) {
            int64_t shamt = base == 4 ? 2 : 5;
            for (size_t opp = max_index - 1, sh_idx = 0; opp >= cur_index; opp--, sh_idx++) {
                char c = toupper(view[opp]);
                if (!isalnum(c)) return total * sign;
                int num = c >= 'A' ? (c - 'A' + 10) : (c - '0');
                if (num >= base) return total;
                total |= static_cast<int64_t>(num) << (shamt * sh_idx);
                if (opp == cur_index) break;
            }
        } else {
            uint64_t pw = 0;
            double base_dbl = static_cast<double>(base);
            double pw_dbl = 0.0;
            for (size_t opp = max_index - 1; opp >= cur_index; opp--) {
                char c = toupper(view[opp]);
                if (!isalnum(c)) return total * sign;
                int num = c >= 'A' ? (c - 'A' + 10) : (c - '0');
                if (num >= base) return total;
                if (is_constant_evaluated()) {
                    total += num * constexpr_int_nonneg_pow(base, pw);
                    pw++;
                } else {
                    total += static_cast<int64_t>(round((num * pow(base_dbl, pw_dbl))));
                    pw_dbl += 1.0;
                }
                if (opp == cur_index) break;
            }
        }
        return total * sign;
    }

    constexpr int StrViewToInt(const StringView view, int base = 10) {
        return static_cast<int>(StrViewToI64(view, base));
    }
    constexpr int64_t StrToI64(const String& str, int base = 10) { return StrViewToI64(str.view(), base); }
    constexpr int StrToInt(const String& str, int base = 10) {
        return static_cast<int>(StrViewToI64(str.view(), base));
    }

#ifdef COMPILER_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#elif COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#endif

    template <SupportedBase Base = SupportedBase::Decimal>
    String IntToStr(Integral auto value) {
        if constexpr (Base == SupportedBase::Decimal) {
            if constexpr (IsSigned<decltype(value)>) {
                using Ut = MakeUnsignedT<decltype(value)>;
                const bool neg = value < 0;
                const auto uvalue = neg ? static_cast<Ut>(~value) + static_cast<Ut>(1) : static_cast<Ut>(value);
                const auto len = StrLenFromIntegral<Ut>(uvalue);
                String result{len + (neg ? 1 : 0), '-'};
                WriteToCharsImpl(result.rawptr() + neg, len, uvalue);
                return result;
            } else {
                size_t len = StrLenFromIntegral(value);
                String result{len, '\0'};
                WriteToCharsImpl(result.rawptr(), len, value);
                return result;
            }
        } else {
            String rev{};
            if constexpr (Base == SupportedBase::Hexadecimal) {
                if (value == 0) rev.append('0');
                while (value > 0) {
                    Integral auto rem = value % 16;
                    if (rem > 9)
                        rev.append(static_cast<char>(rem) + '7');
                    else
                        rev.append(static_cast<char>(rem) + '0');
                    value >>= 4;
                }
                rev.append('x');
                rev.append('0');
            } else if constexpr (Base == SupportedBase::Binary) {
                if (value == 0) rev.append('0');
                while (value > 0) {
                    Integral auto rem = value % 2;
                    rev.append(static_cast<char>(rem) + '0');
                    value >>= 1;
                }
                rev.append('b');
                rev.append('0');
            }
            return rev.reversed();
        }
    }

#ifdef COMPILER_GCC
#pragma GCC diagnostic pop
#elif COMPILER_CLANG
#pragma clang diagnostic pop
#endif

    inline bool StrToBool(const String& value) {
        constexpr StringView view{"true"};
        if (value == view)
            return true;
        else
            return false;
    }

    inline String BoolToStr(bool value) {
        if (value) return String{"true"};
        return String{"false"};
    }

    inline String CharToStr(char value) { return String{1, value}; }

    inline String ToString(Stringable auto& value) {
        if constexpr (IsSameV<decltype(value), String>) return value;
        return value.to_string();
    }

    BASIC_PRINT_IMPL(short, IntToStr)
    BASIC_PRINT_IMPL(unsigned short, IntToStr)
    BASIC_PRINT_IMPL(int, IntToStr)
    BASIC_PRINT_IMPL(unsigned int, IntToStr<SupportedBase::Hexadecimal>);
    BASIC_PRINT_IMPL(long, IntToStr)
    BASIC_PRINT_IMPL(unsigned long, IntToStr)
    BASIC_PRINT_IMPL(long long, IntToStr)
    BASIC_PRINT_IMPL(unsigned long long, IntToStr)
    BASIC_PRINT_IMPL(long double, LongDoubleToStr);
    BASIC_PRINT_IMPL(double, DoubleToStr)
    BASIC_PRINT_IMPL(float, FloatToStr)
    BASIC_PRINT_IMPL(unsigned char, IntToStr<SupportedBase::Hexadecimal>);
    BASIC_PRINT_IMPL(char, CharToStr);
    BASIC_PRINT_IMPL(bool, BoolToStr)
} // namespace ARLib

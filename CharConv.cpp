#include "CharConv.h"
#include "Assertion.h"
#include "StringView.h"
#include "Vector.h"
#include <charconv>
namespace ARLib {
Result<uint64_t> StrViewToU64(const StringView view, int base) {
    if (base < 2 || base > 36)
        return "Failed to convert stringview to integer, invalid base (valid bases are 2 to 36)"_s;

    size_t cur_index = 0;
    size_t max_index = view.length();

    // skip leading and trailing whitespace
    if (max_index == 0) return "Failed to convert stringview to integer, empty string"_s;

    while (isspace(view[cur_index])) {
        cur_index++;
        if (cur_index == max_index) return "Failed to convert stringview to integer, empty string"_s;
    }

    while (isspace(view[max_index - 1])) {
        max_index--;
        if (max_index == npos_) return "Failed to convert stringview to integer, empty string"_s;
    }

    if (cur_index == max_index) return "Failed to convert stringview to integer, empty string"_s;

    switch (base) {
        case 2:
            return StrViewToU64Binary(view.substringview(cur_index, max_index));
        case 8:
            return StrViewToU64Octal(view.substringview(cur_index, max_index));
        case 10:
            return StrViewToU64Decimal(view.substringview(cur_index, max_index));
        case 16:
            return StrViewToU64Hexadecimal(view.substringview(cur_index, max_index));
        default:
            // bases other than 2, 8, 10 and 16 take the slow path
            break;
    }

    if (cur_index == max_index) return 0ull;

    // skip leading zeros
    while (view[cur_index] == '0') { cur_index++; }

    if (cur_index == max_index) return 0ull;

    // 0-9 => 48-57
    // A-Z => 65-90

    // base is a power of 2, we can use bitshifts, else, use pow
    // allowed power of 2s are {4, 32}, because base is bounded [2, 36]
    // and 2, 8 and 16 are already handled elsewhere
    uint64_t total = 0;
    if (base == 4 || base == 32) {
        uint64_t shamt = base == 4 ? 2 : 5;
        for (size_t opp = max_index - 1, sh_idx = 0; opp >= cur_index; opp--, sh_idx++) {
            char c = toupper(view[opp]);
            if (!isalnum(c)) return total;
            int num = c >= 'A' ? (c - 'A' + 10) : (c - '0');
            if (num >= base) return total;
            total |= static_cast<uint64_t>(num) << (shamt * sh_idx);
            if (opp == cur_index) break;
        }
    } else {
        uint64_t pw     = 0;
        double base_dbl = static_cast<double>(base);
        double pw_dbl   = 0.0;
        for (size_t opp = max_index - 1; opp >= cur_index; opp--) {
            char c = toupper(view[opp]);
            if (!isalnum(c)) return total;
            int num = c >= 'A' ? (c - 'A' + 10) : (c - '0');
            if (num >= base) return total;
            if (is_constant_evaluated()) {
                total += static_cast<uint64_t>(num * constexpr_int_nonneg_pow(base, pw));
                pw++;
            } else {
                total += static_cast<uint64_t>(round((num * pow(base_dbl, pw_dbl))));
                pw_dbl += 1.0;
            }
            if (opp == cur_index) break;
        }
    }
    return total;
}
Result<int64_t> StrViewToI64(const StringView view, int base) {
    if (base < 2 || base > 36)
        return "Failed to convert stringview to integer, invalid base (valid bases are 2 to 36)"_s;

    size_t cur_index = 0;
    size_t max_index = view.length();

    // skip leading and trailing whitespace
    if (max_index == 0) return "Failed to convert stringview to integer, empty string"_s;

    while (isspace(view[cur_index])) {
        cur_index++;
        if (cur_index == max_index) return "Failed to convert stringview to integer, empty string"_s;
    }

    while (isspace(view[max_index - 1])) {
        max_index--;
        if (max_index == npos_) return "Failed to convert stringview to integer, empty string"_s;
    }

    if (cur_index == max_index) return "Failed to convert stringview to integer, empty string"_s;

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
    char s   = view[cur_index];
    if (s == '+' || s == '-') {
        sign = s == '+' ? 1 : -1;
        cur_index++;
    }

    if (cur_index == max_index) return 0ll * sign;

    // skip leading zeros
    while (view[cur_index] == '0') { cur_index++; }

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
            total |= static_cast<int64_t>(num) << (static_cast<size_t>(shamt) * sh_idx);
            if (opp == cur_index) break;
        }
    } else {
        uint64_t pw     = 0;
        double base_dbl = static_cast<double>(base);
        double pw_dbl   = 0.0;
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
// FIXME: Write a routine that actually works
// using the stdlib for now because this is too hard to get right
// one day I'll actual write it properly
Result<double> StrViewToDouble(const StringView str) {
    double val                = 0.0;
    auto res = std::from_chars(str.data(), str.data() + str.size(), val);
    if (res.ec == std::errc::invalid_argument) {
        return "Failed to convert string to double, invalid argument"_s;
    } else if (res.ec == std::errc::result_out_of_range) {
        return "Failed to convert string to double, argument out of range"_s;
    }
    return val;
}
Result<float> StrViewToFloat(const StringView str) {
    TRY_SET(val, StrViewToDouble(str));
    return static_cast<float>(val);
}
Result<double> StrToDouble(const String& str) {
    return StrViewToDouble(str.view());
}
Result<float> StrToFloat(const String& str) {
    return StrViewToFloat(str.view());
}
String DoubleToStrImpl(double value, const char* fmt, int precision) {
    const auto len = static_cast<size_t>(scprintf(fmt, value)) + static_cast<size_t>(precision);
    String str{};
    str.reserve(len);
    char fmtc  = ARLib::tolower(fmt[1]);
    bool upper = fmtc != fmt[1];
    std::chars_format char_format[]{ std::chars_format::scientific, std::chars_format::fixed,
                                     std::chars_format::general };
    char* ptr    = str.rawptr();
    auto ec      = std::to_chars(ptr, ptr + len, value, char_format[fmtc - 'e'], precision);
    size_t wrlen = static_cast<size_t>(ec.ptr - str.rawptr());
    str.set_size(wrlen);
    return upper ? str.upper() : str;
}
String LongDoubleToStrImpl(long double value, const char* fmt, int precision) {
    const auto len = static_cast<size_t>(scprintf(fmt, value)) + static_cast<size_t>(precision);
    String str{};
    str.reserve(len);
    char fmtc  = ARLib::tolower(fmt[2]);
    bool upper = fmtc != fmt[2];
    std::chars_format char_format[]{ std::chars_format::scientific, std::chars_format::fixed,
                                     std::chars_format::general };
    char* ptr    = str.rawptr();
    auto ec      = std::to_chars(ptr, ptr + len, value, char_format[fmtc - 'e'], precision);
    size_t wrlen = static_cast<size_t>(ec.ptr - str.rawptr());
    str.set_size(wrlen);
    return upper ? str.upper() : str;
}
String FloatToStrImpl(float value, const char* fmt, int precision) {
    return DoubleToStrImpl(static_cast<double>(value), fmt, precision);
}
}    // namespace ARLib

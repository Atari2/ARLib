#include "PrintfImpl.h"
#include "Array.h"
#include "Assertion.h"
#include "CharConv.h"
#include "EnumHelpers.h"
#include "Printer.h"
#include "Result.h"
#include "SSOVector.h"
#include "StringView.h"
#include "TypeTraits.h"
#include "Utility.h"
#include "cmath_compat.h"
#include "cstdio_compat.h"
#include "arlib_osapi.h"
#include "WStringView.h"
#include <stdarg.h>
namespace ARLib {
/*
    double-precision	sign bit, 11-bit exponent, 52-bit significand
    assumes iec559 compliance
    */

struct DoubleRepr {
    bool sign;
    int16_t exp;
    uint64_t signif;
};
static DoubleRepr double_to_bits(FloatingPoint auto num_orig) {
    // %A specifies `double` precision only
    double num = static_cast<double>(num_orig);
    static_assert(sizeof(double) == sizeof(uint64_t));
    uint64_t val = 0;
    ARLib::memcpy(&val, &num, sizeof(double));
    constexpr uint64_t highest_bitmask = 1_u64 << 63_u64;
    constexpr uint64_t exp_bitmask     = 0b0111111111110000000000000000000000000000000000000000000000000000;
    constexpr uint64_t signif_bitmask  = 0b0000000000001111111111111111111111111111111111111111111111111111;
    bool sign                          = (val & highest_bitmask);
    int16_t exp                        = static_cast<int16_t>(((val & exp_bitmask) >> 52_u64) - 1023_i16);
    uint64_t signif                    = val & signif_bitmask;
    return { .sign = sign, .exp = exp, .signif = signif };
}
// https://learn.microsoft.com/en-us/cpp/c-runtime-library/format-specification-syntax-printf-and-wprintf-functions?view=msvc-170
namespace PrintfTypes {
    enum class Type : char {
        None              = 0,
        CharSingle        = 'c',
        CharWide          = 'C',
        SignedDecimal1    = 'd',
        SignedDecimal2    = 'i',
        UnsignedOctal     = 'o',
        UnsignedDecimal   = 'u',
        UnsignedHexLower  = 'x',
        UnsignedHexUpper  = 'X',
        FloatExp          = 'e',
        FloatExpUpper     = 'E',
        Float             = 'f',
        FloatUpper        = 'F',
        FloatCompact      = 'g',
        FloatCompactUpper = 'G',
        FloatHex          = 'a',
        FloatHexUpper     = 'A',
        NumOfChars        = 'n',
        Pointer           = 'p',
        String            = 's',
        WideString        = 'S',
        AnsiString        = 'Z'
    };
    enum class Flags : uint8_t {
        None         = 0x00,
        LeftAlign    = 0x01,
        UseSign      = 0x02,
        LeadingZeros = 0x04,
        Blank        = 0x08,
        AddPrefix    = 0x10
    };
    // field: %[flags][width][.precision][size]type
    enum class AllowedToParse : uint8_t {
        None        = 0x00,
        Flags       = 0x01,
        Width       = 0x02,
        Precision   = 0x04,
        Size        = 0x08,
        Type        = 0x10,
        All         = 0x01 | 0x02 | 0x04 | 0x08 | 0x10,
        NoFlags     = 0x02 | 0x04 | 0x08 | 0x10,
        NoWidth     = 0x04 | 0x08 | 0x10,
        NoPrecision = 0x08 | 0x10
    };
    MAKE_BITFIELD_ENUM(AllowedToParse);
    MAKE_BITFIELD_ENUM(Flags);
    enum class Size : uint8_t {
        Char,
        Short,
        Int32,
        Int64,
        IntMax,
        Long,
        LongDouble,
        LongLong,
        PtrDiff,
        Size,
        Wide,
        Missing,
        Invalid
    };
    static_assert(sizeof(Type) == 1, "Type must be 1 byte");
    static_assert(sizeof(Flags) == 1, "Flags must be 1 byte");
    static_assert(sizeof(Size) == 1, "Size must be 1 byte");
    struct PrintfInfo {
        constexpr static inline size_t variable_width_marker     = 0xCACABABACACABABA;
        constexpr static inline size_t variable_precision_marker = 0xDADAEAEADADAEAEA;
        constexpr static inline size_t missing_precision_marker  = 0xDEDEFEFEDEDEFEFE;
        Type type                                                = Type::None;
        Flags flags                                              = Flags::None;
        Size size                                                = Size::Missing;
        bool is_escape                                           = false;
        size_t width                                             = NumberTraits<int>::max;
        size_t precision                                         = missing_precision_marker;
        size_t begin_idx                                         = 0;
        size_t end_idx                                           = 0;
    };
}    // namespace PrintfTypes
template <typename T>
concept AllowedFormatArgs = Integral<T> || FloatingPoint<T> || SameAs<T, const char*> || SameAs<T, const wchar_t*> ||
                            SameAs<T, uint64_t*> || SameAs<T, const void*>;
template <FloatFmtOpt Opt>
String RealToStr(FloatingPoint auto val, int precision) {
    using T = RemoveCvRefT<decltype(val)>;
    if constexpr (SameAs<T, float>) {
        return FloatToStr<Opt>(val, precision);
    } else if constexpr (SameAs<T, double>) {
        return DoubleToStr<Opt>(val, precision);
    } else if constexpr (SameAs<T, long double>) {
        return LongDoubleToStr<Opt>(val, precision);
    } else {
        COMPTIME_ASSERT("Invalid type passed to RealToStr");
    }
}
String wchar_to_char(const wchar_t* wstr) {
    return wstring_to_string(WStringView{ wstr });
}
String wchar_to_char(const wchar_t wc) {
    return wstring_to_string(WStringView{ &wc, 1 });
}
template <PrintfTypes::Type... types>
constexpr bool type_is_any_of(PrintfTypes::Type type) {
    constexpr const Array tps{ types... };
    return any_of(tps, [type](auto t) { return t == type; });
}
template <Integral T>
Result<String, PrintfErrorCodes> format_integer_like_type(const PrintfTypes::PrintfInfo& info, T val) {
    using namespace PrintfTypes;
    auto has_flag = [&](Flags flag) {
        return (info.flags & flag) != Flags::None;
    };
    String formatted_arg{};
    switch (info.type) {
        case Type::SignedDecimal1:
        case Type::SignedDecimal2:
        case Type::UnsignedDecimal:
            formatted_arg = IntToStr(val);
            break;
        case Type::UnsignedHexLower:
            formatted_arg = IntToStr<SupportedBase::Hexadecimal>(val).lower();
            break;
        case Type::UnsignedHexUpper:
            formatted_arg = IntToStr<SupportedBase::Hexadecimal>(val).upper();
            break;
        case Type::UnsignedOctal:
            formatted_arg = IntToStr<SupportedBase::Octal>(val);
            break;
        case Type::CharSingle:
            return String{ 1, static_cast<char>(val) };
        case Type::CharWide:
            return wchar_to_char(static_cast<wchar_t>(val));
        default:
            return PrintfErrorCodes::InvalidType;
    }
    const bool is_unsigned =
    type_is_any_of<Type::UnsignedHexLower, Type::UnsignedHexUpper, Type::UnsignedOctal>(info.type);
    const bool is_signed = type_is_any_of<Type::SignedDecimal1, Type::SignedDecimal2>(info.type);
    StringView prefix{ "" };
    if (is_unsigned && has_flag(Flags::AddPrefix)) {
        // '#' => When it's used with the o, x, or X format, the # flag uses 0, 0x, or 0X, respectively, to prefix any nonzero output value.
        switch (info.type) {
            case Type::UnsignedHexLower:
                prefix = "0x"_sv;
                break;
            case Type::UnsignedHexUpper:
                prefix = "0X"_sv;
                break;
            case Type::UnsignedOctal:
                prefix = "0"_sv;
                break;
            default:
                ASSERT_NOT_REACHED("Invalid type passed to format_integer_like_type");
                break;
        }
    }
    // 	The precision has no effect on %c and %C
    if (info.precision != PrintfInfo::missing_precision_marker && (formatted_arg.size() + prefix.size()) < info.precision) {
        // The precision specifies the minimum number of digits to be printed.
        // If the number of digits in the argument is less than precision, the output value is padded on the left with zeros.
        // The value isn't truncated when the number of digits exceeds precision.
        formatted_arg = String{ info.precision - formatted_arg.size() - prefix.size(), '0' } + formatted_arg;
    }
    if (has_flag(Flags::Blank) && is_signed && val > 0 && !has_flag(Flags::UseSign)) {
        // Use a blank to prefix the output value if it's signed and positive. The blank is ignored if both the blank and + flags appear.
        formatted_arg = " "_s + formatted_arg;
    } else if (has_flag(Flags::LeadingZeros) && info.precision == PrintfInfo::missing_precision_marker && !has_flag(Flags::LeftAlign) && (formatted_arg.size() + prefix.size()) < info.width) {
        // If width is prefixed by 0, leading zeros are added until the minimum width is reached.
        // If both 0 and - appear, the 0 is ignored.
        // If 0 is specified for an integer format (i, u, x, X, o, d) and a precision specification is also present-for example, %04.d-the 0 is ignored.
        formatted_arg = String{ info.width - formatted_arg.size() - prefix.size(), '0' } + formatted_arg;
    } else if (info.width != NumberTraits<int>::max && (formatted_arg.size() + prefix.size()) < info.width) {
        // The width argument is a non-negative decimal integer that controls the minimum number of characters that are output.
        // If the number of characters in the output value is less than the specified width, blanks are added to the left or the right of the values-depending
        // on whether the left-alignment flag (-) is specified-until the minimum width is reached.
        if (has_flag(Flags::LeftAlign)) {
            formatted_arg = formatted_arg + String{ info.width - formatted_arg.size() - prefix.size(), ' ' };
        } else {
            formatted_arg = String{ info.width - formatted_arg.size() - prefix.size(), ' ' } + formatted_arg;
        }
    }
    if (!prefix.empty()) { formatted_arg = prefix.extract_string() + formatted_arg; }
    return formatted_arg;
}
template <FloatingPoint T>
Result<String, PrintfErrorCodes> format_real_like_type(PrintfTypes::PrintfInfo& info, T val) {
    // FIXME: adhere to the spec
    using namespace PrintfTypes;
    if (info.precision == PrintfInfo::missing_precision_marker) {
        if (type_is_any_of<Type::FloatHex, Type::FloatHexUpper>(info.type)) {
            // Default precision is 13. If precision is 0, no decimal point is printed unless the #flag is used.
            info.precision = 13;
        } else {
            // Default precision is 6.
            info.precision = 6;
        }
    }
    int precision           = static_cast<int>(info.precision);
    using Vt                = RemoveCvRefT<decltype(val)>;
    constexpr const Vt zero = static_cast<Vt>(0.0);
    auto has_flag           = [&](Flags flag) {
        return (info.flags & flag) != Flags::None;
    };
    auto sign = [&]() {
        auto yes = ((val < T{ 0 }) ? ""_s : "+"_s);
        auto no  = ""_s;
        return has_flag(Flags::UseSign) ? yes : no;
    };

    // '#' 	When it's used with the e, E, f, F, a, or A format, the # flag forces the output value to contain a decimal point.
    // '#' 	When it's used with the g or G format, the # flag forces the output value to contain a decimal point and prevents the truncation of trailing zeros.
    switch (info.type) {
        case Type::FloatExpUpper:
            {
                auto temp        = RealToStr<FloatFmtOpt::E>(val, precision).upper();
                auto exp_idx     = temp.index_of('E');
                auto exp_num     = temp.substring(exp_idx);
                auto without_exp = temp.substring(0, exp_idx);
                auto rest        = without_exp.substring(0, temp.index_of('.') + 1 + info.precision);
                return sign() + rest + exp_num;
            }
        case Type::FloatUpper:
            {
                auto temp = RealToStr<FloatFmtOpt::F>(val, precision).upper();
                return sign() + (temp.substring(0, temp.index_of('.') + 1 + info.precision));
            }
        case Type::FloatCompactUpper:
            {
                auto temp = RealToStr<FloatFmtOpt::G>(val, precision).upper();
                return sign() + (temp.substring(0, temp.index_of('.') + 1_sz + info.precision));
            }
        case Type::FloatExp:
            {
                auto temp        = RealToStr<FloatFmtOpt::e>(val, precision);
                auto exp_idx     = temp.index_of('e');
                auto exp_num     = temp.substring(exp_idx);
                auto without_exp = temp.substring(0, exp_idx);
                auto rest        = without_exp.substring(0, temp.index_of('.') + 1_sz + info.precision);
                return sign() + rest + exp_num;
            }
        case Type::Float:
            {
                auto temp = RealToStr<FloatFmtOpt::f>(val, precision);
                return sign() + (temp.substring(0, temp.index_of('.') + 1_sz + info.precision));
            }
        case Type::FloatCompact:
            {
                auto temp = RealToStr<FloatFmtOpt::g>(val, precision);
                return sign() + (temp.substring(0, temp.index_of('.') + 1_sz + info.precision));
            }
        case Type::FloatHex:
            {
                if (detail::is_nan(val)) {
                    return (has_flag(Flags::UseSign) ? ((val < zero) ? "-"_s : "+"_s) : ((val < zero) ? "-"_s : ""_s)) +
                           "nan"_s;
                } else if (detail::is_infinity(val)) {
                    return (has_flag(Flags::UseSign) ? ((val < zero) ? "-"_s : "+"_s) : ((val < zero) ? "-"_s : ""_s)) +
                           "inf"_s;
                }
                auto [sign_bit, exp, signif] = double_to_bits(val);
                auto builder =
                has_flag(Flags::UseSign) ? (sign_bit ? "-0x1."_s : "+0x1."_s) : (sign_bit ? "-0x1."_s : "0x1."_s);
                builder += IntToStr<SupportedBase::Hexadecimal>(signif).substring(0, info.precision);
                builder += 'p';
                builder += has_flag(Flags::UseSign) ? ((exp < 0_i8) ? ""_s : "+"_s) : ""_s;
                builder += IntToStr(exp);
                return builder;
            }
        case Type::FloatHexUpper:
            {
                if (detail::is_nan(val)) {
                    return (has_flag(Flags::UseSign) ? ((val < zero) ? "-"_s : "+"_s) : ((val < zero) ? "-"_s : ""_s)) +
                           "NAN"_s;
                } else if (detail::is_infinity(val)) {
                    return (has_flag(Flags::UseSign) ? ((val < zero) ? "-"_s : "+"_s) : ((val < zero) ? "-"_s : ""_s)) +
                           "INF"_s;
                }
                auto [sign_bit, exp, signif] = double_to_bits(val);
                auto builder =
                has_flag(Flags::UseSign) ? (sign_bit ? "-0x1."_s : "+0x1."_s) : (sign_bit ? "-0x1."_s : "0x1."_s);
                builder += IntToStr<SupportedBase::Hexadecimal>(signif).substring(0, info.precision);
                builder += 'p';
                builder += has_flag(Flags::UseSign) ? ((exp < 0_i8) ? ""_s : "+"_s) : ""_s;
                builder += IntToStr(exp);
                return builder.upper();
            }
        default:
            return PrintfErrorCodes::InvalidType;
    }
}
template <AllowedFormatArgs T>
Result<String, PrintfErrorCodes> format_single_arg(PrintfTypes::PrintfInfo& info, T val) {
    using namespace PrintfTypes;
    if constexpr (Integral<T>) {
        return format_integer_like_type(info, val);
    } else if constexpr (FloatingPoint<T>) {
        return format_real_like_type(info, val);
    } else if constexpr (SameAs<const char*, T>) {
        if (info.type != Type::String) { return PrintfErrorCodes::InvalidType; }
        return String{ val };
    } else if constexpr (SameAs<const wchar_t*, T>) {
        HARD_ASSERT(info.type == Type::WideString || info.type == Type::AnsiString, "Invalid type");
        if (info.type != Type::WideString && info.type != Type::AnsiString) { return PrintfErrorCodes::InvalidType; }
        return wchar_to_char(val);
    } else if constexpr (SameAs<const void*, T>) {
        constexpr auto ptrlen = sizeof(void*) * 2;
        auto ptrstr           = IntToStr<SupportedBase::Hexadecimal>(BitCast<uintptr_t>(val));
        if (ptrstr.length() < ptrlen) { ptrstr = String{ ptrlen - ptrstr.length(), '0' } + ptrstr; }
        return ptrstr;
    } else {
        COMPTIME_ASSERT("Invalid type passed to format_single_args");
    }
}
PrintfResult printf_impl(PrintfResult& output, const char* fmt, va_list args) {
    using namespace PrintfTypes;
    auto map_c_to_flags = [](char c) {
        switch (c) {
            case '-':
                return Flags::LeftAlign;
            case '+':
                return Flags::UseSign;
            case '0':
                return Flags::LeadingZeros;
            case '#':
                return Flags::AddPrefix;
            case ' ':
                return Flags::Blank;
            default:
                return Flags::None;
        }
    };

    constexpr const Array valid_types{ 'c', 'C', 'd', 'i', 'o', 'u', 'x', 'X', 'e', 'E', 'f',
                                       'F', 'g', 'G', 'a', 'A', 'n', 'p', 's', 'S', 'Z' };
    constexpr const Array valid_flags{ '-', '+', '0', ' ', '#' };
    constexpr const Array valid_sizes{ "hh"_sv, "h"_sv,  "I32"_sv, "I64"_sv, "j"_sv, "l"_sv,
                                       "L"_sv,  "ll"_sv, "t"_sv,   "I"_sv,   "z"_sv, "w"_sv };
    constexpr const Array valid_sizes_map{ Size::Char,    Size::Short, Size::Int32,      Size::Int64,
                                           Size::IntMax,  Size::Long,  Size::LongDouble, Size::LongLong,
                                           Size::PtrDiff, Size::Size,  Size::Size,       Size::Wide };
    static_assert(valid_sizes.size() == valid_sizes_map.size(), "Size maps are different length");

    auto map_str_to_size = [&](StringView str) {
        if (size_t idx = find(valid_sizes, str); idx != npos_) {
            return valid_sizes_map[idx];
        } else {
            return Size::Invalid;
        }
    };
    constexpr const char number_beg = '0';
    constexpr const char number_end = '9';
    auto is_num                     = [](char c) -> bool {
        return c >= number_beg && c <= number_end;
    };

    SSOVector<PrintfInfo> fmtargs{};

#define SET_ERROR(error)                                                                                               \
    output.error_code = error;                                                                                         \
    return output

#define INVALID_FORMAT SET_ERROR(PrintfErrorCodes::InvalidFormat)

    // field: %[flags][width][.precision][size]type
    StringView format{ fmt };
    bool in_format_spec = false;
    PrintfInfo cur_info{};
    AllowedToParse can_parse = AllowedToParse::All;
    for (size_t i = 0; i < format.size() - 1; i++) {
        if (in_format_spec) {
            char cur = format[i];
            if (find_if(valid_flags, [cur, can_parse](const char c) {
                    return c != ' ' ? c == cur :
                                      (c == cur && (can_parse & AllowedToParse::Flags) != AllowedToParse::None);
                }) != npos_) {
                // flags
                if ((can_parse & AllowedToParse::Flags) == AllowedToParse::None) { INVALID_FORMAT; }
                cur_info.flags = cur_info.flags | map_c_to_flags(cur);
                while (find(valid_flags, format[i + 1]) != npos_) {
                    cur_info.flags = cur_info.flags | map_c_to_flags(format[i + 1]);
                    ++i;    // skip
                }
                can_parse = AllowedToParse::NoFlags;
            } else if (is_num(cur) || cur == '*') {
                // possibly width specifier
                if ((can_parse & AllowedToParse::Width) == AllowedToParse::None) { INVALID_FORMAT; }
                if (cur == '*') {
                    cur_info.width = PrintfInfo::variable_width_marker;
                    ++i;
                } else {
                    size_t end_width = i;
                    while (is_num(format[end_width])) ++end_width;
                    int width      = StrViewToInt(format.substringview(i, end_width));
                    cur_info.width = static_cast<size_t>(width);
                    i              = end_width - 1;
                }
                can_parse = AllowedToParse::NoWidth;
            } else if (cur == '.' && (is_num(format[i + 1]) || format[i + 1] == '*')) {
                // possibly precision specifier
                if ((can_parse & AllowedToParse::Precision) == AllowedToParse::None) { INVALID_FORMAT; }
                ++i;
                if (format[i] == '*') {
                    cur_info.precision = PrintfInfo::variable_precision_marker;
                    ++i;
                } else {
                    size_t end_precision = i;
                    while (is_num(format[end_precision])) ++end_precision;
                    int precision      = StrViewToInt(format.substringview(i, end_precision));
                    cur_info.precision = static_cast<size_t>(precision);
                    i                  = end_precision - 1;
                }
                can_parse = AllowedToParse::NoPrecision;
            } else if (find(valid_types, cur) != npos_) {
                if ((can_parse & AllowedToParse::Type) == AllowedToParse::None) { INVALID_FORMAT; }
                // type
                cur_info.type = to_enum<Type>(cur);
                can_parse     = AllowedToParse::None;
            } else {
                if ((can_parse & AllowedToParse::Size) == AllowedToParse::None) {
                    if (cur_info.type == Type::None || ((can_parse & AllowedToParse::Type) != AllowedToParse::None)) {
                        INVALID_FORMAT;
                    }
                    in_format_spec   = false;
                    cur_info.end_idx = i;
                    fmtargs.append(cur_info);
                    cur_info.flags     = Flags::None;
                    cur_info.type      = Type::None;
                    cur_info.precision = PrintfInfo::missing_precision_marker;
                    cur_info.width     = max(0);
                    cur_info.is_escape = false;
                    can_parse          = AllowedToParse::All;
                } else {
                    auto onecharview   = format.substringview_fromlen(i, 1);
                    auto twocharview   = format.substringview_fromlen(i, 2);
                    auto threecharview = format.substringview_fromlen(i, 3);
                    if (auto lsz = map_str_to_size(threecharview); lsz != Size::Invalid) {
                        cur_info.size = lsz;
                        i += 2;
                    } else if (auto tsz = map_str_to_size(twocharview); tsz != Size::Invalid) {
                        cur_info.size = tsz;
                        ++i;
                    } else if (auto osz = map_str_to_size(onecharview); osz != Size::Invalid) {
                        cur_info.size = osz;
                    } else {
                        if (cur_info.type == Type::None || ((can_parse & AllowedToParse::Type) != AllowedToParse::None)) {
                            SET_ERROR(PrintfErrorCodes::MissingType);
                        }
                        in_format_spec   = false;
                        cur_info.end_idx = i;
                        fmtargs.append(cur_info);
                        cur_info.flags     = Flags::None;
                        cur_info.type      = Type::None;
                        cur_info.precision = PrintfInfo::missing_precision_marker;
                        cur_info.width     = max(0);
                        cur_info.is_escape = false;
                        can_parse          = AllowedToParse::All;
                    }
                }
            }
        }
        if (format[i] == '%') {
            if (format[i + 1] == '%') {
                // escaped, increase i and continue
                PrintfInfo escapeinfo{ .is_escape = true, .begin_idx = i, .end_idx = i + 2 };
                fmtargs.append(escapeinfo);
                ++i;
                continue;
            } else {
                in_format_spec     = true;
                cur_info.begin_idx = i;
            }
        }
    }

    if (cur_info.type != Type::None) {
        cur_info.end_idx = format.size() - 1;
        fmtargs.append(cur_info);
    } else if (in_format_spec && (can_parse & AllowedToParse::Type) != AllowedToParse::None) {
        // last char may be a valid type
        cur_info.end_idx = format.size();
        cur_info.type    = to_enum<Type>(format[format.size() - 1]);
        fmtargs.append(cur_info);
    }

    output.reserve(format.size());
    size_t prev_idx = 0;
    String formatted_arg;

    auto append_if_correct = [&](PrintfInfo& info, AllowedFormatArgs auto val) {
        auto res = format_single_arg(info, val);
        if (res.is_ok()) {
            formatted_arg = res.to_ok();
            return PrintfErrorCodes::Ok;
        } else {
            return res.to_error();
        }
    };

#define APPEND_AND_RET_IF_FAIL(type, promotion_type)                                                                   \
    {                                                                                                                  \
        type vvar = static_cast<type>(va_arg(args, promotion_type));                                                   \
        auto ec   = append_if_correct(fdesc, vvar);                                                                    \
        if (ec != PrintfErrorCodes::Ok) {                                                                              \
            SET_ERROR(ec);                                                                                             \
        } else {                                                                                                       \
            output.written_arguments++;                                                                                \
        }                                                                                                              \
    }

    for (auto& fdesc : fmtargs) {
        formatted_arg.clear();
        output += format.substringview(prev_idx, fdesc.begin_idx).extract_string();
        prev_idx = fdesc.end_idx;
        using enum Type;
        if (fdesc.is_escape) {
            output += '%';
            continue;
        }
        if (fdesc.width == PrintfInfo::variable_width_marker) { fdesc.width = static_cast<size_t>(va_arg(args, int)); }
        if (fdesc.precision == PrintfInfo::variable_precision_marker) {
            fdesc.precision = static_cast<size_t>(va_arg(args, int));
        }
        switch (fdesc.type) {
            case CharSingle:
                APPEND_AND_RET_IF_FAIL(char, int);
                break;
            case CharWide:
                APPEND_AND_RET_IF_FAIL(wchar_t, int);
                break;
            case SignedDecimal1:
            case SignedDecimal2:
                {
                    switch (fdesc.size) {
                        case Size::Char:
                            APPEND_AND_RET_IF_FAIL(int8_t, int);
                            break;
                        case Size::Short:
                            APPEND_AND_RET_IF_FAIL(int16_t, int);
                            break;
                        case Size::Missing:
                        case Size::Int32:
                            APPEND_AND_RET_IF_FAIL(int32_t, int);
                            break;
                        case Size::Int64:
                        case Size::IntMax:
                        case Size::Long:
                        case Size::LongLong:
                            APPEND_AND_RET_IF_FAIL(int64_t, int64_t);
                            break;
                        case Size::PtrDiff:
                            APPEND_AND_RET_IF_FAIL(ptrdiff_t, ptrdiff_t);
                            break;
                        case Size::Size:
                            APPEND_AND_RET_IF_FAIL(size_t, size_t);
                            break;
                        case Size::Wide:
                        case Size::LongDouble:
                        case Size::Invalid:
                        default:
                            INVALID_FORMAT;
                    }
                }
                break;
            case UnsignedOctal:
            case UnsignedDecimal:
            case UnsignedHexLower:
            case UnsignedHexUpper:
                {
                    switch (fdesc.size) {
                        case Size::Char:
                            APPEND_AND_RET_IF_FAIL(uint8_t, uint32_t);
                            break;
                        case Size::Short:
                            APPEND_AND_RET_IF_FAIL(uint16_t, uint32_t);
                            break;
                        case Size::Missing:
                        case Size::Int32:
                            APPEND_AND_RET_IF_FAIL(uint32_t, uint32_t);
                            break;
                        case Size::Int64:
                        case Size::IntMax:
                        case Size::Long:
                        case Size::LongLong:
                            APPEND_AND_RET_IF_FAIL(uint64_t, uint64_t);
                            break;
                        case Size::PtrDiff:
                            APPEND_AND_RET_IF_FAIL(ptrdiff_t, ptrdiff_t);
                            break;
                        case Size::Size:
                            APPEND_AND_RET_IF_FAIL(size_t, size_t);
                            break;
                        case Size::Wide:
                        case Size::LongDouble:
                        case Size::Invalid:
                        default:
                            INVALID_FORMAT;
                    }
                }
                break;
            case FloatExp:
            case FloatExpUpper:
            case Float:
            case FloatUpper:
            case FloatCompact:
            case FloatCompactUpper:
            case FloatHex:
            case FloatHexUpper:
                switch (fdesc.size) {
                    case Size::Missing:
                        APPEND_AND_RET_IF_FAIL(double, double);
                        break;
                    case Size::LongDouble:
                        APPEND_AND_RET_IF_FAIL(long double, long double);
                        break;
                    default:
                        INVALID_FORMAT;
                }
                break;
            case NumOfChars:
                switch (fdesc.size) {
                    case Size::Char:
                        {
                            int8_t* var = va_arg(args, int8_t*);
                            *var        = static_cast<int8_t>(output.size());
                        }
                        break;
                    case Size::Short:
                        {
                            int16_t* var = va_arg(args, int16_t*);
                            *var         = static_cast<int16_t>(output.size());
                        }
                        break;
                    case Size::Missing:
                    case Size::Int32:
                        {
                            int32_t* var = va_arg(args, int32_t*);
                            *var         = static_cast<int32_t>(output.size());
                        }
                        break;
                    case Size::Int64:
                    case Size::IntMax:
                    case Size::Long:
                    case Size::LongLong:
                    case Size::Size:
                    case Size::PtrDiff:
                        {
                            int64_t* var = va_arg(args, int64_t*);
                            *var         = static_cast<int64_t>(output.size());
                        }
                        break;
                    case Size::Wide:
                    case Size::LongDouble:
                    case Size::Invalid:
                    default:
                        INVALID_FORMAT;
                }
                break;
            case String:
                APPEND_AND_RET_IF_FAIL(const char*, const char*);
                break;
            case WideString:
            case AnsiString:
                APPEND_AND_RET_IF_FAIL(const wchar_t*, const wchar_t*);
                break;
            case Pointer:
                APPEND_AND_RET_IF_FAIL(const void*, const void*);
                break;
            default:
                ASSERT_NOT_REACHED("Invalid format specifier!");
                break;
        }
        output += formatted_arg;
    }
    output += format.substringview(prev_idx).extract_string();
    return output;
}
PrintfResult _vsprintf(_In_z_ _Printf_format_string_ const char* fmt, va_list args) {
    PrintfResult result{ PrintfResultType::FromString, String{}, {}, {} };
    return printf_impl(result, fmt, args);
}
PrintfResult _vsprintf_frombuf(_In_z_ _Printf_format_string_ const char* fmt, va_list args, PrintfBuffer buffer) {
    PrintfResult result{ PrintfResultType::FromBuffer, move(buffer), {}, {} };
    return printf_impl(result, fmt, args);
}
void PrintfResult::reserve(size_t size) {
    if (type == PrintfResultType::FromString) { result.get<String>().reserve(size); }
}
PrintfResult& PrintfResult::operator+=(const String& other) {
    if (type == PrintfResultType::FromString) {
        result.get<String>() += other;
    } else {
        auto& buffer = result.get<PrintfBuffer>();
        auto rem     = buffer.buffer_size - buffer.written_size;
        if (rem > 0) {
            ARLib::strncpy(buffer.buffer + buffer.written_size, other.data(), min_bt(rem, other.size()));
            buffer.written_size += other.size();
        }
    }
    return *this;
}
PrintfResult& PrintfResult::operator+=(char other) {
    if (type == PrintfResultType::FromString) {
        result.get<String>() += other;
    } else {
        auto& buffer = result.get<PrintfBuffer>();
        auto rem     = buffer.buffer_size - buffer.written_size;
        if (rem > 0) {
            buffer.buffer[buffer.written_size] = other;
            buffer.written_size += 1;
        }
    }
    return *this;
}
size_t PrintfResult::size() const {
    if (type == PrintfResultType::FromString) {
        return result.get<String>().size();
    } else {
        return result.get<PrintfBuffer>().written_size;
    }
}
const char* PrintfResult::data() const {
    if (type == PrintfResultType::FromString) {
        return result.get<String>().data();
    } else {
        return result.get<PrintfBuffer>().buffer;
    }
}
void PrintfResult::finalize() {
    if (type == PrintfResultType::FromBuffer) {
        auto& buffer                       = result.get<PrintfBuffer>();
        buffer.buffer[buffer.written_size] = '\0';
    }
}
}    // namespace ARLib

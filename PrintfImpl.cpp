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
    uint64_t val                 = 0;
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
    enum class Flags : uint8_t { None = 0x00, LeftAlign = 0x01, UseSign = 0x02, LeadingZeros = 0x04, AddPrefix = 0x08 };
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
        Type type        = Type::None;
        Flags flags      = Flags::None;
        Size size        = Size::Missing;
        bool is_escape   = false;
        size_t width     = NumberTraits<int>::max;
        size_t precision = 6;
        size_t begin_idx = 0;
        size_t end_idx   = 0;
    };
}    // namespace PrintfTypes
template <typename T>
concept AllowedFormatArgs =
Integral<T> || FloatingPoint<T> || SameAs<T, const char*> || SameAs<T, const wchar_t*> || SameAs<T, uint64_t*> || SameAs<T, const void*>;
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
    size_t i = 0;
    String output{};
    while (wstr[i] != L'\0') { 
        wchar_t v = wstr[i];
        if (v >= 0x00 && v <= 0x7F) { 
            output.append(static_cast<char>(v));
        } else {
            output.append('?');
        }
        ++i;
    }
    return output;
}
String wchar_to_char(const wchar_t wc) {
    String output{};
    if (wc >= 0x00 && wc <= 0x7F) {
        output.append(static_cast<char>(wc));
    } else {
        output.append('?');
    }
    return output;
}
template <AllowedFormatArgs T>
Result<String, PrintfErrorCodes> format_single_arg(const PrintfTypes::PrintfInfo& info, T val) {
#define HAS_FLAG(flag, yes, no) (((info.flags & Flags::flag) != Flags::None) ? (yes) : (no))
#define SIGN                    HAS_FLAG(UseSign, (val < 0 ? ""_s : "+"_s), ""_s)
#define WANTS_PREFIX(base)                                                                                             \
    (HAS_FLAG(AddPrefix, (IntToStr<SupportedBase::base, true>(val)), (IntToStr<SupportedBase::base, false>(val))))
    using namespace PrintfTypes;
    if constexpr (Integral<T>) {
        switch (info.type) {
            case Type::SignedDecimal1:
            case Type::SignedDecimal2:
                return SIGN + IntToStr(val);
            case Type::UnsignedDecimal:
                return IntToStr(val);
            case Type::UnsignedHexLower:
                return WANTS_PREFIX(Hexadecimal).lower();
            case Type::UnsignedHexUpper:
                return WANTS_PREFIX(Hexadecimal).upper();
            case Type::UnsignedOctal:
                return WANTS_PREFIX(Octal);
            case Type::CharSingle:
                return String{ 1, static_cast<char>(val) };
            case Type::CharWide:
                return wchar_to_char(static_cast<wchar_t>(val));
            default:
                return PrintfErrorCodes::InvalidType;
        }
    } else if constexpr (FloatingPoint<T>) {
        int precision = static_cast<int>(info.precision);
        using Vt = RemoveCvRefT<decltype(val)>;
        constexpr const Vt zero = static_cast<Vt>(0.0);
        switch (info.type) {
            case Type::FloatExpUpper:
                {
                    auto temp        = RealToStr<FloatFmtOpt::E>(val, precision).upper();
                    auto exp_idx     = temp.index_of('E');
                    auto exp_num     = temp.substring(exp_idx);
                    auto without_exp = temp.substring(0, exp_idx);
                    auto rest        = without_exp.substring(0, temp.index_of('.') + 1 + info.precision);
                    return SIGN + rest + exp_num;
                }
            case Type::FloatUpper:
                {
                    auto temp = RealToStr<FloatFmtOpt::F>(val, precision).upper();
                    return SIGN + (temp.substring(0, temp.index_of('.') + 1 + info.precision));
                }
            case Type::FloatCompactUpper:
                {
                    auto temp = RealToStr<FloatFmtOpt::G>(val, precision).upper();
                    return SIGN + (temp.substring(0, temp.index_of('.') + 1_sz + info.precision));
                }
            case Type::FloatExp:
                {
                    auto temp        = RealToStr<FloatFmtOpt::e>(val, precision);
                    auto exp_idx     = temp.index_of('e');
                    auto exp_num     = temp.substring(exp_idx);
                    auto without_exp = temp.substring(0, exp_idx);
                    auto rest        = without_exp.substring(0, temp.index_of('.') + 1_sz + info.precision);
                    return SIGN + rest + exp_num;
                }
            case Type::Float:
                {
                    auto temp = RealToStr<FloatFmtOpt::f>(val, precision);
                    return SIGN + (temp.substring(0, temp.index_of('.') + 1_sz + info.precision));
                }
            case Type::FloatCompact:
                {
                    auto temp = RealToStr<FloatFmtOpt::g>(val, precision);
                    return SIGN + (temp.substring(0, temp.index_of('.') + 1_sz + info.precision));
                }
            case Type::FloatHex:
                {
                    if (detail::is_nan(val)) {
                        return HAS_FLAG(UseSign, (val < zero) ? "-"_s : "+"_s, (val < zero) ? "-"_s : ""_s) + "nan"_s;
                    } else if (detail::is_infinity(val)) {
                        return HAS_FLAG(UseSign, (val < zero) ? "-"_s : "+"_s, (val < zero) ? "-"_s : ""_s) + "inf"_s;
                    }
                    auto [sign, exp, signif] = double_to_bits(val);
                    auto builder = HAS_FLAG(UseSign, sign ? "-0x1."_s : "+0x1."_s, sign ? "-0x1."_s : "0x1."_s);
                    builder += IntToStr<SupportedBase::Hexadecimal>(signif).substring(0, info.precision);
                    builder += 'p';
                    builder += HAS_FLAG(UseSign, (exp < 0_i8) ? ""_s : "+"_s, ""_s);
                    builder += IntToStr(exp);
                    return builder;
                }
            case Type::FloatHexUpper:
                {
                    if (detail::is_nan(val)) {
                        return HAS_FLAG(UseSign, (val < zero) ? "-"_s : "+"_s, (val < zero) ? "-"_s : ""_s) + "NAN"_s;
                    } else if (detail::is_infinity(val)) {
                        return HAS_FLAG(UseSign, (val < zero) ? "-"_s : "+"_s, (val < zero) ? "-"_s : ""_s) + "INF"_s;
                    }
                    auto [sign, exp, signif] = double_to_bits(val);
                    auto builder = HAS_FLAG(UseSign, sign ? "-0x1."_s : "+0x1."_s, sign ? "-0x1."_s : "0x1."_s);
                    builder += IntToStr<SupportedBase::Hexadecimal>(signif).substring(0, info.precision);
                    builder += 'p';
                    builder += HAS_FLAG(UseSign, (exp < 0_i8) ? ""_s : "+"_s, ""_s);
                    builder += IntToStr(exp);
                    return builder.upper();
                }
            default:
                return PrintfErrorCodes::InvalidType;
        }
    } else if constexpr (SameAs<const char*, T>) {
        if (info.type != Type::String) { return PrintfErrorCodes::InvalidType; }
        return String{ val };
    } else if constexpr (SameAs<const wchar_t*, T>) {
        HARD_ASSERT(info.type == Type::WideString || info.type == Type::AnsiString, "Invalid type");
        if (info.type != Type::WideString && info.type != Type::AnsiString) { return PrintfErrorCodes::InvalidType; }
        return wchar_to_char(val);
    } else if constexpr (SameAs<const void*, T>) {
        constexpr auto ptrlen = sizeof(void*) * 2;
        auto ptrstr = IntToStr<SupportedBase::Hexadecimal>(BitCast<uintptr_t>(val));
        if (ptrstr.length() < ptrlen) {
            ptrstr = String{ ptrlen - ptrstr.length(),  '0' } + ptrstr;
        }
        return ptrstr;
    }else {
        COMPTIME_ASSERT("Invalid type passed to format_single_args");
    }
}
PrintfResult printf_impl(const char* fmt, va_list args) {
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
            default:
                return Flags::None;
        }
    };

    constexpr const Array valid_types{ 'c', 'C', 'd', 'i', 'o', 'u', 'x', 'X', 'e', 'E', 'f',
                                       'F', 'g', 'G', 'a', 'A', 'n', 'p', 's', 'S', 'Z' };
    constexpr const Array valid_flags{ '-', '+', '0', '#' };
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

    PrintfResult result{};

#define SET_ERROR(error)                                                                                               \
    result.error_code = error;                                                                                         \
    return result

#define INVALID_FORMAT SET_ERROR(PrintfErrorCodes::InvalidFormat)

    // field: %[flags][width][.precision][size]type
    StringView format{ fmt };
    bool in_format_spec = false;
    PrintfInfo cur_info{};
    AllowedToParse can_parse = AllowedToParse::All;
    for (size_t i = 0; i < format.size() - 1; i++) {
        if (in_format_spec) {
            char cur = format[i];
            if (find(valid_flags, cur) != npos_) {
                // flags
                if ((can_parse & AllowedToParse::Flags) == AllowedToParse::None) { INVALID_FORMAT; }
                cur_info.flags = cur_info.flags | map_c_to_flags(cur);
                while (find(valid_flags, format[i + 1]) != npos_) {
                    cur_info.flags = cur_info.flags | map_c_to_flags(format[i + 1]);
                    ++i;    // skip
                }
                can_parse = AllowedToParse::NoFlags;
            } else if (is_num(cur)) {
                // possibly width specifier
                if ((can_parse & AllowedToParse::Width) == AllowedToParse::None) { INVALID_FORMAT; }
                size_t end_width = i;
                while (is_num(format[end_width])) ++end_width;
                int width      = StrViewToInt(format.substringview(i, end_width));
                cur_info.width = static_cast<size_t>(width);
                i              = end_width - 1;
                can_parse      = AllowedToParse::NoWidth;
            } else if (cur == '.' && is_num(format[i + 1])) {
                // possibly precision specifier
                if ((can_parse & AllowedToParse::Precision) == AllowedToParse::None) { INVALID_FORMAT; }
                ++i;
                size_t end_precision = i;
                while (is_num(format[end_precision])) ++end_precision;
                int precision      = StrViewToInt(format.substringview(i, end_precision));
                cur_info.precision = static_cast<size_t>(precision);
                i                  = end_precision - 1;
                can_parse          = AllowedToParse::NoPrecision;
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
                    cur_info.precision = 6;
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
                        cur_info.precision = 6;
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

    String& output = result.result;
    output.reserve(format.size());
    size_t prev_idx = 0;
    String formatted_arg;

    auto append_if_correct = [&](const PrintfInfo& info, AllowedFormatArgs auto val) {
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
            result.written_arguments++;                                                                                \
        }                                                                                                              \
    }

    for (const auto& fdesc : fmtargs) {
        formatted_arg.clear();
        output += format.substringview(prev_idx, fdesc.begin_idx).extract_string();
        prev_idx = fdesc.end_idx;
        using enum Type;
        if (fdesc.is_escape) {
            output += '%';
            continue;
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
    return result;
}
PrintfResult _vsprintf(_In_z_ _Printf_format_string_ const char* fmt, va_list args) {
    return printf_impl(fmt, args);
}
}    // namespace ARLib
#include "PrintfImpl.h"
#include "Array.h"
#include "EnumHelpers.h"
#include "Printer.h"
#include "SSOVector.h"
#include "StringView.h"
#include <cstdarg>

namespace ARLib {
    // https://learn.microsoft.com/en-us/cpp/c-runtime-library/format-specification-syntax-printf-and-wprintf-functions?view=msvc-170
    namespace PrintfTypes {
        enum class Type : char {
            None = 0,
            CharSingle = 'c',
            CharWide = 'C',
            SignedDecimal1 = 'd',
            SignedDecimal2 = 'i',
            UnsignedOctal = 'o',
            UnsignedDecimal = 'u',
            UnsignedHexLower = 'x',
            UnsignedHexUpper = 'X',
            FloatExp = 'e',
            FloatExpUpper = 'E',
            Float = 'f',
            FloatUpper = 'F',
            FloatCompact = 'g',
            FloatCompatUpper = 'G',
            FloatHex = 'a',
            FloatHexUpper = 'A',
            NumOfChars = 'n',
            String = 's',
            WideString = 'S',
            AnsiString = 'Z'
        };
        enum class Flags : char {
            None = 0x00,
            LeftAlign = 0x01,
            UseSign = 0x02,
            LeadingZeros = 0x04,
            AddPrefix = 0x08
        };
        // field: %[flags][width][.precision][size]type
        enum class AllowedToParse : uint32_t {
            None = 0x00,
            Flags = 0x01,
            Width = 0x02,
            Precision = 0x04,
            Size = 0x08,
            Type = 0x10,
            All = 0x01 | 0x02 | 0x04 | 0x08 | 0x10,
            NoFlags = 0x02 | 0x04 | 0x08 | 0x10,
            NoWidth = 0x04 | 0x08 | 0x10,
            NoPrecision = 0x08 | 0x10
        };
        MAKE_BITFIELD_ENUM(AllowedToParse);
        MAKE_BITFIELD_ENUM(Flags);
        enum class Size { Char, Short, Int32, Int64, IntMax, Long, LongDouble, LongLong, PtrDiff, Size, Wide, Invalid };
        struct PrintfInfo {
            Type type = Type::None;
            Flags flags = Flags::None;
            Size size = Size::Invalid;
            int width = NumberTraits<int>::max;
            int precision = NumberTraits<int>::max;
            size_t begin_idx = 0;
            size_t end_idx = 0;
            bool is_escape = false;
        };
    } // namespace PrintfTypes

    template <typename T>
    concept AllowedFormatArgs =
    Integral<T> || FloatingPoint<T> || SameAs<T, const char*> || SameAs<T, const wchar_t*> || SameAs<T, uint64_t*>;

    template <AllowedFormatArgs T>
    String format_single_arg(const PrintfTypes::PrintfInfo& info, T val) {
#define HAS_FLAG(flag, yes, no) (((info.flags & Flags::flag) != Flags::None) ? yes : no)
#define SIGN                    HAS_FLAG(UseSign, (val < 0 ? "-"_s : "+"_s), ""_s)
#define WANTS_PREFIX(base)                                                                                             \
    (HAS_FLAG(AddPrefix, (IntToStr<SupportedBase::base, true>(val)), (IntToStr<SupportedBase::base, false>(val))))
        using namespace PrintfTypes;
        if constexpr (Integral<T>) {
            switch (info.type) {
            case Type::SignedDecimal1:
            case Type::SignedDecimal2:
            case Type::UnsignedDecimal:
                return SIGN + IntToStr(val);
            case Type::UnsignedHexLower:
                return SIGN + WANTS_PREFIX(Hexadecimal).lower();
            case Type::UnsignedHexUpper:
                return SIGN + WANTS_PREFIX(Hexadecimal).upper();
            case Type::UnsignedOctal:
                return SIGN + WANTS_PREFIX(Octal);
            case Type::CharSingle:
                return String{1, static_cast<char>(val)};
            case Type::CharWide:
                return "Unsupported CharWide"_s;
            default:
                return "Wrong type for integral"_s;
            }
        } else if constexpr (FloatingPoint<T>) {
            switch (info.type) {
            case Type::FloatExpUpper:
            case Type::FloatUpper:
            case Type::FloatCompatUpper: {
                auto temp = DoubleToStr(val).upper();
                return SIGN + (temp.substring(0, temp.index_of('.') + 1 + info.precision));
            }
            case Type::FloatExp:
            case Type::Float:
            case Type::FloatCompact: {
                auto temp = DoubleToStr(val);
                return SIGN + (temp.substring(0, temp.index_of('.') + 1 + info.precision));
            }
            case Type::FloatHex:
            case Type::FloatHexUpper:
                return "Unsupported float hex format"_s;
            default:
                return "Wrong type for floating point"_s;
            }
        } else if constexpr (SameAs<const char*, T>) {
            HARD_ASSERT(info.type == Type::String, "Invalid type");
            return String{val};
        } else if constexpr (SameAs<const wchar_t*, T>) {
            HARD_ASSERT(info.type == Type::WideString || info.type == Type::AnsiString, "Invalid type");
            return "Wide strings not supported"_s;
        } else if constexpr (SameAs<uint64_t*, T>) {
            return "Number of characters to pointer not supported"_s;
        } else {
            COMPTIME_ASSERT("Invalid type passed to format_single_args");
        }
    }

    PrintfErrorCodes printf_impl(const char* fmt, ...) {
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

        static constexpr size_t invalid_fmt = static_cast<size_t>(-1);
        constexpr const Array valid_types{'c', 'C', 'd', 'i', 'o', 'u', 'x', 'X', 'e', 'E', 'f',
                                          'F', 'g', 'G', 'a', 'A', 'n', 'P', 's', 'S', 'Z'};
        constexpr const Array valid_flags{'-', '+', '0', '#'};
        constexpr const Array valid_sizes{"hh"_sv, "h"_sv,  "I32"_sv, "I64"_sv, "j"_sv, "l"_sv,
                                          "L"_sv,  "ll"_sv, "t"_sv,   "I"_sv,   "z"_sv, "w"_sv};
        constexpr const Array valid_sizes_map{Size::Char,    Size::Short, Size::Int32,      Size::Int64,
                                              Size::IntMax,  Size::Long,  Size::LongDouble, Size::LongLong,
                                              Size::PtrDiff, Size::Size,  Size::Size,       Size::Wide};
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
        auto is_num = [](char c) -> bool {
            return c >= number_beg && c <= number_end;
        };
        constexpr auto numbers = "0123456789"_sv;

        SSOVector<PrintfInfo> fmtargs{};

        // field: %[flags][width][.precision][size]type
        va_list args{};
        size_t arg_idx = 0;
        va_start(args, fmt);
        StringView format{fmt};
        bool in_format_spec = false;
        PrintfInfo cur_info{};
        AllowedToParse can_parse = AllowedToParse::All;
        for (size_t i = 0; i < format.size() - 1; i++) {
            if (in_format_spec) {
                char cur = format[i];
                if (find(valid_flags, cur) != npos_) {
                    // flags
                    if ((can_parse & AllowedToParse::Flags) == AllowedToParse::None) {
                        return PrintfErrorCodes::InvalidFormat;
                    }
                    cur_info.flags = cur_info.flags | map_c_to_flags(cur);
                    while (find(valid_flags, format[i + 1]) != npos_) {
                        cur_info.flags = cur_info.flags | map_c_to_flags(format[i + 1]);
                        ++i; // skip
                    }
                    can_parse = AllowedToParse::NoFlags;
                } else if (is_num(cur)) {
                    // possibly width specifier
                    if ((can_parse & AllowedToParse::Width) == AllowedToParse::None) {
                        return PrintfErrorCodes::InvalidFormat;
                    }
                    size_t end_width = i;
                    while (is_num(format[end_width]))
                        ++end_width;
                    int width = StrViewToInt(format.substringview(i, end_width));
                    cur_info.width = width;
                    i = end_width - 1;
                    can_parse = AllowedToParse::NoWidth;
                } else if (cur == '.' && is_num(format[i + 1])) {
                    // possibly precision specifier
                    if ((can_parse & AllowedToParse::Precision) == AllowedToParse::None) {
                        return PrintfErrorCodes::InvalidFormat;
                    }
                    ++i;
                    size_t end_precision = i;
                    while (is_num(format[end_precision]))
                        ++end_precision;
                    int precision = StrViewToInt(format.substringview(i, end_precision));
                    cur_info.precision = precision;
                    i = end_precision - 1;
                    can_parse = AllowedToParse::NoPrecision;
                } else if (find(valid_types, cur) != npos_) {
                    if ((can_parse & AllowedToParse::Type) == AllowedToParse::None) {
                        return PrintfErrorCodes::InvalidFormat;
                    }
                    // type
                    cur_info.type = to_enum<Type>(cur);
                    can_parse = AllowedToParse::None;
                } else {
                    if ((can_parse & AllowedToParse::Size) == AllowedToParse::None) {
                        if (cur_info.type == Type::None ||
                            ((can_parse & AllowedToParse::Type) != AllowedToParse::None)) {
                            return PrintfErrorCodes::MissingType;
                        }
                        in_format_spec = false;
                        cur_info.end_idx = i;
                        fmtargs.append(cur_info);
                        cur_info.flags = Flags::None;
                        cur_info.type = to_enum<Type>(0);
                        cur_info.precision = 0;
                        cur_info.width = 0;
                        can_parse = AllowedToParse::All;
                    } else {
                        auto onecharview = format.substringview_fromlen(i, 1);
                        auto twocharview = format.substringview_fromlen(i, 2);
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
                            if (cur_info.type == Type::None ||
                                ((can_parse & AllowedToParse::Type) != AllowedToParse::None)) {
                                return PrintfErrorCodes::MissingType;
                            }
                            in_format_spec = false;
                            cur_info.end_idx = i;
                            fmtargs.append(cur_info);
                            cur_info.flags = Flags::None;
                            cur_info.type = to_enum<Type>(0);
                            cur_info.precision = 0;
                            cur_info.width = 0;
                            can_parse = AllowedToParse::All;
                        }
                    }
                }
            }
            if (format[i] == '%') {
                if (format[i + 1] == '%') {
                    // escaped, increase i and continue
                    PrintfInfo escapeinfo{.begin_idx = i, .end_idx = i + 2, .is_escape = true};
                    fmtargs.append(escapeinfo);
                    ++i;
                    continue;
                } else {
                    in_format_spec = true;
                    cur_info.begin_idx = i;
                }
            }
        }
        String output{};
        output.reserve(format.size());
        size_t prev_idx = 0;
        for (const auto& fdesc : fmtargs) {
            output += format.substringview(prev_idx, fdesc.begin_idx).extract_string();
            prev_idx = fdesc.end_idx;
            String formatted_arg;
            using enum Type;
            if (fdesc.is_escape) {
                output += '%';
                continue;
            }
            switch (fdesc.type) {
            case CharSingle:
                formatted_arg = format_single_arg(fdesc, va_arg(args, char));
                break;
            case CharWide:
                formatted_arg = format_single_arg(fdesc, va_arg(args, wchar_t));
                break;
            case SignedDecimal1:
            case SignedDecimal2:
                formatted_arg = format_single_arg(fdesc, va_arg(args, int64_t));
                break;
            case UnsignedOctal:
            case UnsignedDecimal:
            case UnsignedHexLower:
            case UnsignedHexUpper:
                formatted_arg = format_single_arg(fdesc, va_arg(args, uint64_t));
                break;
            case FloatExp:
            case FloatExpUpper:
            case Float:
            case FloatUpper:
            case FloatCompact:
            case FloatCompatUpper:
            case FloatHex:
            case FloatHexUpper:
                formatted_arg = format_single_arg(fdesc, va_arg(args, double));
                break;
            case NumOfChars:
                formatted_arg = format_single_arg(fdesc, va_arg(args, uint64_t*));
                break;
            case String:
                formatted_arg = format_single_arg(fdesc, va_arg(args, const char*));
                break;
            case WideString:
            case AnsiString:
                formatted_arg = format_single_arg(fdesc, va_arg(args, const wchar_t*));
                break;
            default:
                break;
            }
            output += formatted_arg;
        }
        output += format.substringview(prev_idx).extract_string();
        // ARLib::fwrite(output.data(), 1, output.size(), stdout);
        va_end(args);
        return PrintfErrorCodes::Ok;
    }

} // namespace ARLib
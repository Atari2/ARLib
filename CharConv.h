#pragma once
#include "Assertion.h"
#include "Concepts.h"
#include "PrintInfo.h"
#include "StringView.h"
#include "cmath_compat.h"
#include "cstdio_compat.h"

namespace ARLib {

    enum class SupportedBase { Decimal, Hexadecimal, Binary, Octal };

    // FIXME: avoid pow/round calls in Str{View}To{I64/int}
    // TODO: make StrToInt more efficient
    // add StrToUInt
    int64_t StrViewToI64(StringView view, int base = 10);
    int StrViewToInt(StringView view, int base = 10);
    int64_t StrToI64(const String& str, int base = 10);
    int StrToInt(const String& str, int base = 10);
    // FIXME: make this more efficient
    double StrToDouble(const String& str);
    float StrToFloat(const String& str);
    String DoubleToStr(double value);
    String LongDoubleToStr(long double value);
    String FloatToStr(float value);

    constexpr size_t StrLenFromIntegral(Integral auto value) noexcept {
        static_assert(!IsSigned<decltype(value)>, "Value must be unsigned");
        size_t n = 1;
        constexpr size_t base = 10;
        constexpr size_t b2 = base * base;
        constexpr size_t b3 = b2 * base;
        constexpr size_t b4 = b3 * base;
        for (;;) {
            if (value < static_cast<decltype(value)>(base)) return n;
            if (value < b2) return n + 1;
            if (value < b3) return n + 2;
            if (value < b4) return n + 3;
            value /= b4;
            n += 4;
        }
    }

    void WriteToCharsImpl(char* ptr, size_t len, Integral auto value) {
        static constexpr char digits[201] = "0001020304050607080910111213141516171819"
                                            "2021222324252627282930313233343536373839"
                                            "4041424344454647484950515253545556575859"
                                            "6061626364656667686970717273747576777879"
                                            "8081828384858687888990919293949596979899";
        auto pos = len - 1;
        while (value >= 100) {
            auto const num = (value % 100) * 2;
            value /= 100;
            ptr[pos] = digits[num + 1];
            ptr[pos - 1] = digits[num];
            pos -= 2;
        }
        if (value >= 10) {
            auto const num = value * 2;
            ptr[1] = digits[num + 1];
            ptr[0] = digits[num];
        } else
            ptr[0] = '0' + static_cast<char>(value);
    }

    template <SupportedBase Base = SupportedBase::Decimal>
    String IntToStr(Integral auto value) {
        if constexpr (Base == SupportedBase::Decimal) {
            if constexpr (IsSigned<decltype(value)>) {
                using Ut = MakeUnsignedT<decltype(value)>;
                const bool neg = value < 0;
                const auto uvalue = neg ? static_cast<Ut>(~value) + 1ul : static_cast<Ut>(value);
                const auto len = StrLenFromIntegral<Ut>(uvalue);
                String result{neg + len, '-'};
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

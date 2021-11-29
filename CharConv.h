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

    template <SupportedBase Base = SupportedBase::Decimal>
    String IntToStr(Integral auto value) {
        if constexpr (Base == SupportedBase::Decimal) {
            char buf[22] = {0};
            char* const buf_end = end(buf) - 1;
            char* next = buf_end;
            const auto uvalue = static_cast<MakeUnsignedT<decltype(value)>>(value);
            if (value < 0) {
                next = unsigned_to_buffer(next, 0 - uvalue);
                *--next = '-';
            } else {
                next = unsigned_to_buffer(next, uvalue);
            }
            return String{next, buf_end};
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

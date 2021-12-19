#pragma once
#include "Assertion.h"
#include "Concepts.h"
#include "PrintInfo.h"
#include "StringView.h"
#include "cmath_compat.h"
#include "cstdio_compat.h"

namespace ARLib {
    // these functions expect an already whitespace trimmed string
    constexpr int64_t StrViewToI64Decimal(const StringView str) {
        size_t start_idx = 0;
        size_t end_idx = str.size();
        char maybe_sign = str[0];
        bool neg = maybe_sign == '-';
        start_idx += (maybe_sign == '-' || maybe_sign == '+');
        int64_t result = 0;

        // skip leading zeros
        while (start_idx < end_idx) {
            if (str[start_idx] != '0') break;
            start_idx++;
        }
        
        if (start_idx == end_idx) return 0;

        // out of range check
        constexpr size_t max_int64_size = strlen("9223372036854775807");
        if (end_idx - start_idx > max_int64_size) return 0;

        constexpr int64_t values_table[max_int64_size] = {1,
                                                          10,
                                                          100,
                                                          1000,
                                                          10000,
                                                          100000,
                                                          1000000,
                                                          10000000,
                                                          100000000,
                                                          1000000000,
                                                          10000000000,
                                                          100000000000,
                                                          1000000000000,
                                                          10000000000000,
                                                          100000000000000,
                                                          1000000000000000,
                                                          10000000000000000,
                                                          100000000000000000,
                                                          1000000000000000000};
        // 0-9 => 48-57
        for (size_t idx = end_idx - 1, tbl_idx = 0; idx >= start_idx; idx--, tbl_idx++) {
            result += static_cast<int64_t>(str[idx] - '0') * values_table[tbl_idx];
            if (idx == 0) break;
        }
        return result * (neg ? -1 : 1);
    }
    constexpr int64_t StrViewToI64Binary(const StringView str) {
        size_t start_idx = 0;
        size_t end_idx = str.size();
        char maybe_sign = str[0];
        bool neg = maybe_sign == '-';
        start_idx += (maybe_sign == '-' || maybe_sign == '+');
        int64_t result = 0;

        // skip b if string is e.g. 0b11
        if (str[start_idx] == '0' && str[start_idx + 1] == 'b') start_idx += 2;

        // skip leading zeros
        while (start_idx < end_idx) {
            if (str[start_idx] != '0') break;
            start_idx++;
        }

        if (start_idx == end_idx) return 0;

        // out of range check
        constexpr size_t max_int64_size = strlen("111111111111111111111111111111111111111111111111111111111111111");
        if (end_idx - start_idx > max_int64_size) return 0;
        // 0-1 => 48-49
        for (size_t idx = end_idx - 1, tbl_idx = 0; idx >= start_idx; idx--, tbl_idx++) {
            result |= static_cast<int64_t>(str[idx] - '0') << tbl_idx;
            if (idx == 0) break;
        }
        return result * (neg ? -1 : 1);
    }
    constexpr int64_t StrViewToI64Octal(const StringView str) {
        size_t start_idx = 0;
        size_t end_idx = str.size();
        char maybe_sign = str[0];
        bool neg = maybe_sign == '-';
        start_idx += (maybe_sign == '-' || maybe_sign == '+');
        int64_t result = 0;

        // skip b if string is e.g. 0o11
        if (str[start_idx] == '0' && str[start_idx + 1] == 'o') start_idx += 2;

        // skip leading zeros
        while (start_idx < end_idx) {
            if (str[start_idx] != '0') break;
            start_idx++;
        }

        if (start_idx == end_idx) return 0;

        // out of range check
        constexpr size_t max_int64_size = strlen("777777777777777777777");
        if (end_idx - start_idx > max_int64_size) return 0;
        // 0-1 => 48-49
        for (size_t idx = end_idx - 1, tbl_idx = 0; idx >= start_idx; idx--, tbl_idx++) {
            result |= static_cast<int64_t>(str[idx] - '0') << (tbl_idx * 3);
            if (idx == 0) break;
        }
        return result * (neg ? -1 : 1);
    }
    constexpr int64_t StrViewToI64Hexadecimal(const StringView str) {
        size_t start_idx = 0;
        size_t end_idx = str.size();
        char maybe_sign = str[0];
        bool neg = maybe_sign == '-';
        start_idx += (maybe_sign == '-' || maybe_sign == '+');
        int64_t result = 0;

        // skip b if string is e.g. 0x11
        if (str[start_idx] == '0' && str[start_idx + 1] == 'x') start_idx += 2;

        // skip leading zeros
        while (start_idx < end_idx) {
            if (str[start_idx] != '0') break;
            start_idx++;
        }

        if (start_idx == end_idx) return 0;

        // out of range check
        constexpr size_t max_int64_size = strlen("7fffffffffffffff");
        if (end_idx - start_idx > max_int64_size) return 0;
        // 0-1 => 48-49
        for (size_t idx = end_idx - 1, tbl_idx = 0; idx >= start_idx; idx--, tbl_idx++) {
            char c = toupper(str[idx]);
            auto num = c >= 'A' ? (c - 'A' + 10) : (c - '0');
            result |= static_cast<int64_t>(num) << (tbl_idx * 4);
            if (idx == 0) break;
        }
        return result * (neg ? -1 : 1);
    }

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

    constexpr void WriteToCharsImpl(char* ptr, size_t len, Integral auto value) {
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
} // namespace ARLib
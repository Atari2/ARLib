#pragma once
#include "Assertion.hpp"
#include "Concepts.hpp"
#include "PrintInfo.hpp"
#include "StringView.hpp"
#include "cmath_compat.hpp"
#include "cstdio_compat.hpp"
#include "Result.hpp"
namespace ARLib {
namespace detail {
    // clang forced my hand
    constexpr static inline int64_t values_table_i64[] = { 1,
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
                                                           1000000000000000000 };
    constexpr static inline uint64_t value_table_u64[] = { 1,
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
                                                           1000000000000000000,
                                                           10000000000000000000ull };
}    // namespace detail
#ifdef COMPILER_GCC
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wconversion"
#endif
// these functions expect an already whitespace trimmed string
template <bool Signed = true, typename RetType = ConditionalT<Signed, int64_t, uint64_t>>
Result<RetType> StrViewTo64Decimal(const StringView str) {
    size_t start_idx = 0;
    size_t end_idx   = str.size();

    if (str.empty()) return "Failed to convert string to integer, string was empty"_s;

    bool neg = false;
    if constexpr (Signed) {
        char maybe_sign = str[0];
        neg             = maybe_sign == '-';
        start_idx += (maybe_sign == '-' || maybe_sign == '+');
    }

    RetType result = 0;

    // skip leading zeros
    while (start_idx < end_idx) {
        if (str[start_idx] != '0') break;
        start_idx++;
    }

    if (start_idx == end_idx) return RetType{ 0 };
    constexpr size_t max_uint64_size = strlen("18446744073709551615");
    constexpr size_t max_int64_size  = strlen("9223372036854775807");

    // out of range check
    auto get_max_size = [&]() {
        if (Signed)
            return max_int64_size;
        else
            return max_uint64_size;
    };

    constexpr auto max_size = get_max_size();

    if (end_idx - start_idx > max_size) return "Failed to convert string to integer, out of range"_s;

    constexpr auto get_values_table = [&]() -> const RetType(&)[max_size] {
        if constexpr (Signed)
            return detail::values_table_i64;
        else
            return detail::value_table_u64;
    };

    constexpr const RetType(&values_table)[max_size] = get_values_table();

    // 0-9 => 48-57
    for (size_t idx = end_idx - 1, tbl_idx = 0; idx >= start_idx; idx--, tbl_idx++) {
        auto c = str[idx];
        if (c < '0' || c > '9') { return "Failed to convert string to integer, invalid decimal character"_s; }
        result += static_cast<RetType>(c - '0') * values_table[tbl_idx];
        if (idx == 0) break;
    }
    if constexpr (Signed)
        return result * (neg ? -1 : 1);
    else
        return result;
}
template <bool Signed = true, typename RetType = ConditionalT<Signed, int64_t, uint64_t>>
Result<RetType> StrViewTo64Binary(const StringView str) {
    size_t start_idx = 0;
    size_t end_idx   = str.size();

    bool neg = false;
    if constexpr (Signed) {
        char maybe_sign = str[0];
        neg             = maybe_sign == '-';
        start_idx += (maybe_sign == '-' || maybe_sign == '+');
    }
    RetType result = 0;

    // skip b if string is e.g. 0b11
    if (str[start_idx] == '0' && str[start_idx + 1] == 'b') start_idx += 2;

    // skip leading zeros
    while (start_idx < end_idx) {
        if (str[start_idx] != '0') break;
        start_idx++;
    }

    if (start_idx == end_idx) return RetType{ 0 };

    // out of range check
    constexpr size_t max_int64_size  = strlen("111111111111111111111111111111111111111111111111111111111111111");
    constexpr size_t max_uint64_size = strlen("1111111111111111111111111111111111111111111111111111111111111111");
    auto get_max_size                = [&]() {
        if (Signed)
            return max_int64_size;
        else
            return max_uint64_size;
    };

    constexpr auto max_size = get_max_size();

    if (end_idx - start_idx > max_size) return "Failed to convert string to integer, out of range"_s;
    // 0-1 => 48-49
    for (size_t idx = end_idx - 1, tbl_idx = 0; idx >= start_idx; idx--, tbl_idx++) {
        auto c = str[idx];
        if (c != '0' && c != '1') { return "Failed to convert string to integer, invalid binary character"_s; }
        result |= static_cast<RetType>(c - '0') << tbl_idx;
        if (idx == 0) break;
    }
    if constexpr (Signed)
        return result * (neg ? -1 : 1);
    else
        return result;
}
template <bool Signed = true, typename RetType = ConditionalT<Signed, int64_t, uint64_t>>
Result<RetType> StrViewTo64Octal(const StringView str) {
    size_t start_idx = 0;
    size_t end_idx   = str.size();
    bool neg         = false;

    if constexpr (Signed) {
        char maybe_sign = str[0];
        neg             = maybe_sign == '-';
        start_idx += (maybe_sign == '-' || maybe_sign == '+');
    }
    RetType result = 0;

    // skip b if string is e.g. 0o11
    if (str[start_idx] == '0' && str[start_idx + 1] == 'o') start_idx += 2;

    // skip leading zeros
    while (start_idx < end_idx) {
        if (str[start_idx] != '0') break;
        start_idx++;
    }

    if (start_idx == end_idx) return RetType{ 0 };

    // out of range check
    constexpr size_t max_int64_size  = strlen("777777777777777777777");
    constexpr size_t max_uint64_size = strlen("1777777777777777777777");

    auto get_max_size = [&]() {
        if (Signed)
            return max_int64_size;
        else
            return max_uint64_size;
    };

    constexpr auto max_size = get_max_size();

    if (end_idx - start_idx > max_size) return "Failed to convert string to integer, out of range"_s;
    // 0-1 => 48-49
    for (size_t idx = end_idx - 1, tbl_idx = 0; idx >= start_idx; idx--, tbl_idx++) {
        auto c = str[idx];
        if (c < '0' || c > '7') { return "Failed to convert string to integer, invalid octal character"_s; }
        result |= static_cast<RetType>(c - '0') << (tbl_idx * 3);
        if (idx == 0) break;
    }
    if constexpr (Signed)
        return result * (neg ? -1 : 1);
    else
        return result;
}
template <bool Signed = true, typename RetType = ConditionalT<Signed, int64_t, uint64_t>>
Result<RetType> StrViewTo64Hexadecimal(const StringView str) {
    size_t start_idx = 0;
    size_t end_idx   = str.size();
    bool neg         = false;
    if constexpr (Signed) {
        char maybe_sign = str[0];
        neg             = maybe_sign == '-';
        start_idx += (maybe_sign == '-' || maybe_sign == '+');
    }
    RetType result = 0;

    // skip b if string is e.g. 0x11
    if (str[start_idx] == '0' && str[start_idx + 1] == 'x') start_idx += 2;

    // skip leading zeros
    while (start_idx < end_idx) {
        if (str[start_idx] != '0') break;
        start_idx++;
    }

    if (start_idx == end_idx) return RetType{ 0 };

    // out of range check
    constexpr size_t max_int64_size  = strlen("7fffffffffffffff");
    constexpr size_t max_uint64_size = strlen("ffffffffffffffff");

    static_assert(max_int64_size == max_uint64_size);

    auto get_max_size = [&]() {
        return max_uint64_size;
    };

    constexpr auto max_size = get_max_size();

    if (end_idx - start_idx > max_size) return "Failed to convert string to integer, out of range"_s;
    // 0-1 => 48-49
    for (size_t idx = end_idx - 1, tbl_idx = 0; idx >= start_idx; idx--, tbl_idx++) {
        char c   = toupper(str[idx]);
        auto num = c >= 'A' ? (c - 'A' + 10) : (c - '0');
        if (num < 0 || num > 15) { return "Failed to convert string to integer, invalid hex character"_s; }
        result |= static_cast<RetType>(num) << static_cast<RetType>(tbl_idx * 4ull);
        if (idx == 0) break;
    }
    if constexpr (Signed)
        return result * (neg ? -1 : 1);
    else
        return result;
}
inline auto StrViewToI64Decimal(const StringView str) {
    return StrViewTo64Decimal<true>(str);
}
inline auto StrViewToI64Binary(const StringView str) {
    return StrViewTo64Binary<true>(str);
}
inline auto StrViewToI64Octal(const StringView str) {
    return StrViewTo64Octal<true>(str);
}
inline auto StrViewToI64Hexadecimal(const StringView str) {
    return StrViewTo64Hexadecimal<true>(str);
}
inline auto StrViewToU64Decimal(const StringView str) {
    return StrViewTo64Decimal<false>(str);
}
inline auto StrViewToU64Binary(const StringView str) {
    return StrViewTo64Binary<false>(str);
}
inline auto StrViewToU64Octal(const StringView str) {
    return StrViewTo64Octal<false>(str);
}
inline auto StrViewToU64Hexadecimal(const StringView str) {
    return StrViewTo64Hexadecimal<false>(str);
}
template <size_t Base>
constexpr size_t StrLenFromIntegral(Integral auto v) noexcept {
    static_assert(!IsSigned<decltype(v)>, "Value must be unsigned");
    size_t value          = static_cast<size_t>(v);
    size_t n              = 1;
    constexpr size_t base = Base;
    constexpr size_t b2   = base * base;
    constexpr size_t b3   = b2 * base;
    constexpr size_t b4   = b3 * base;
    for (;;) {
        if (value < base) return n;
        if (value < b2) return n + 1;
        if (value < b3) return n + 2;
        if (value < b4) return n + 3;
        value /= b4;
        n += 4;
    }
}
constexpr void WriteToCharsImpl(char* ptr, size_t len, Integral auto value) {
    constexpr char digits[201] = "0001020304050607080910111213141516171819"
                                 "2021222324252627282930313233343536373839"
                                 "4041424344454647484950515253545556575859"
                                 "6061626364656667686970717273747576777879"
                                 "8081828384858687888990919293949596979899";
    auto pos                   = len - 1;
    while (value >= 100) {
        const auto num = (value % 100) * 2;
        value /= 100;
        ptr[pos]     = digits[num + 1];
        ptr[pos - 1] = digits[num];
        pos -= 2;
    }
    if (value >= 10) {
        const auto num = value * 2;
        ptr[1]         = digits[num + 1];
        ptr[0]         = digits[num];
    } else
        ptr[0] = '0' + static_cast<char>(value);
}
namespace cxpr {
    template <bool Signed = true, typename RetType = ConditionalT<Signed, int64_t, uint64_t>>
    constexpr RetType StrViewTo64Decimal(const StringView str) {
        size_t start_idx = 0;
        size_t end_idx   = str.size();

        if (str.empty()) return 0;

        bool neg = false;
        if constexpr (Signed) {
            char maybe_sign = str[0];
            neg             = maybe_sign == '-';
            start_idx += (maybe_sign == '-' || maybe_sign == '+');
        }

        RetType result = 0;

        // skip leading zeros
        while (start_idx < end_idx) {
            if (str[start_idx] != '0') break;
            start_idx++;
        }

        if (start_idx == end_idx) return 0;
        constexpr size_t max_uint64_size = strlen("18446744073709551615");
        constexpr size_t max_int64_size  = strlen("9223372036854775807");

        // out of range check
        auto get_max_size = [&]() {
            if (Signed)
                return max_int64_size;
            else
                return max_uint64_size;
        };

        constexpr auto max_size = get_max_size();

        if (end_idx - start_idx > max_size) return 0;

        constexpr auto get_values_table = [&]() -> const RetType(&)[max_size] {
            if constexpr (Signed)
                return detail::values_table_i64;
            else
                return detail::value_table_u64;
        };

        constexpr const RetType(&values_table)[max_size] = get_values_table();

        // 0-9 => 48-57
        for (size_t idx = end_idx - 1, tbl_idx = 0; idx >= start_idx; idx--, tbl_idx++) {
            result += static_cast<RetType>(str[idx] - '0') * values_table[tbl_idx];
            if (idx == 0) break;
        }
        if constexpr (Signed)
            return result * (neg ? -1 : 1);
        else
            return result;
    }
    template <bool Signed = true, typename RetType = ConditionalT<Signed, int64_t, uint64_t>>
    constexpr RetType StrViewTo64Binary(const StringView str) {
        size_t start_idx = 0;
        size_t end_idx   = str.size();

        bool neg = false;
        if constexpr (Signed) {
            char maybe_sign = str[0];
            neg             = maybe_sign == '-';
            start_idx += (maybe_sign == '-' || maybe_sign == '+');
        }
        RetType result = 0;

        // skip b if string is e.g. 0b11
        if (str[start_idx] == '0' && str[start_idx + 1] == 'b') start_idx += 2;

        // skip leading zeros
        while (start_idx < end_idx) {
            if (str[start_idx] != '0') break;
            start_idx++;
        }

        if (start_idx == end_idx) return 0;

        // out of range check
        constexpr size_t max_int64_size  = strlen("111111111111111111111111111111111111111111111111111111111111111");
        constexpr size_t max_uint64_size = strlen("1111111111111111111111111111111111111111111111111111111111111111");
        auto get_max_size                = [&]() {
            if (Signed)
                return max_int64_size;
            else
                return max_uint64_size;
        };

        constexpr auto max_size = get_max_size();

        if (end_idx - start_idx > max_size) return 0;
        // 0-1 => 48-49
        for (size_t idx = end_idx - 1, tbl_idx = 0; idx >= start_idx; idx--, tbl_idx++) {
            result |= static_cast<RetType>(str[idx] - '0') << tbl_idx;
            if (idx == 0) break;
        }
        if constexpr (Signed)
            return result * (neg ? -1 : 1);
        else
            return result;
    }
    template <bool Signed = true, typename RetType = ConditionalT<Signed, int64_t, uint64_t>>
    constexpr RetType StrViewTo64Octal(const StringView str) {
        size_t start_idx = 0;
        size_t end_idx   = str.size();
        bool neg         = false;

        if constexpr (Signed) {
            char maybe_sign = str[0];
            neg             = maybe_sign == '-';
            start_idx += (maybe_sign == '-' || maybe_sign == '+');
        }
        RetType result = 0;

        // skip b if string is e.g. 0o11
        if (str[start_idx] == '0' && str[start_idx + 1] == 'o') start_idx += 2;

        // skip leading zeros
        while (start_idx < end_idx) {
            if (str[start_idx] != '0') break;
            start_idx++;
        }

        if (start_idx == end_idx) return 0;

        // out of range check
        constexpr size_t max_int64_size  = strlen("777777777777777777777");
        constexpr size_t max_uint64_size = strlen("1777777777777777777777");

        auto get_max_size = [&]() {
            if (Signed)
                return max_int64_size;
            else
                return max_uint64_size;
        };

        constexpr auto max_size = get_max_size();

        if (end_idx - start_idx > max_size) return 0;
        // 0-1 => 48-49
        for (size_t idx = end_idx - 1, tbl_idx = 0; idx >= start_idx; idx--, tbl_idx++) {
            result |= static_cast<RetType>(str[idx] - '0') << (tbl_idx * 3);
            if (idx == 0) break;
        }
        if constexpr (Signed)
            return result * (neg ? -1 : 1);
        else
            return result;
    }
    template <bool Signed = true, typename RetType = ConditionalT<Signed, int64_t, uint64_t>>
    constexpr RetType StrViewTo64Hexadecimal(const StringView str) {
        size_t start_idx = 0;
        size_t end_idx   = str.size();
        bool neg         = false;
        if constexpr (Signed) {
            char maybe_sign = str[0];
            neg             = maybe_sign == '-';
            start_idx += (maybe_sign == '-' || maybe_sign == '+');
        }
        RetType result = 0;

        // skip b if string is e.g. 0x11
        if (str[start_idx] == '0' && str[start_idx + 1] == 'x') start_idx += 2;

        // skip leading zeros
        while (start_idx < end_idx) {
            if (str[start_idx] != '0') break;
            start_idx++;
        }

        if (start_idx == end_idx) return 0;

        // out of range check
        constexpr size_t max_int64_size  = strlen("7fffffffffffffff");
        constexpr size_t max_uint64_size = strlen("ffffffffffffffff");

        static_assert(max_int64_size == max_uint64_size);

        auto get_max_size = [&]() {
            return max_uint64_size;
        };

        constexpr auto max_size = get_max_size();

        if (end_idx - start_idx > max_size) return 0;
        // 0-1 => 48-49
        for (size_t idx = end_idx - 1, tbl_idx = 0; idx >= start_idx; idx--, tbl_idx++) {
            char c   = toupper(str[idx]);
            auto num = c >= 'A' ? (c - 'A' + 10) : (c - '0');
            result |= static_cast<RetType>(num) << static_cast<RetType>(tbl_idx * 4ull);
            if (idx == 0) break;
        }
        if constexpr (Signed)
            return result * (neg ? -1 : 1);
        else
            return result;
    }
    arlib_forceinline constexpr auto StrViewToI64Decimal(const StringView str) {
        return cxpr::StrViewTo64Decimal<true>(str);
    }
    arlib_forceinline constexpr auto StrViewToI64Binary(const StringView str) {
        return cxpr::StrViewTo64Binary<true>(str);
    }
    arlib_forceinline constexpr auto StrViewToI64Octal(const StringView str) {
        return cxpr::StrViewTo64Octal<true>(str);
    }
    arlib_forceinline constexpr auto StrViewToI64Hexadecimal(const StringView str) {
        return cxpr::StrViewTo64Hexadecimal<true>(str);
    }
    arlib_forceinline constexpr auto StrViewToU64Decimal(const StringView str) {
        return cxpr::StrViewTo64Decimal<false>(str);
    }
    arlib_forceinline constexpr auto StrViewToU64Binary(const StringView str) {
        return cxpr::StrViewTo64Binary<false>(str);
    }
    arlib_forceinline constexpr auto StrViewToU64Octal(const StringView str) {
        return cxpr::StrViewTo64Octal<false>(str);
    }
    arlib_forceinline constexpr auto StrViewToU64Hexadecimal(const StringView str) {
        return cxpr::StrViewTo64Hexadecimal<false>(str);
    }
}    // namespace cxpr
#ifdef COMPILER_GCC
    #pragma GCC diagnostic pop
#endif
}    // namespace ARLib

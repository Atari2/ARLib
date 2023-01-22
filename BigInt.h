#pragma once

#include "CharConv.h"
#include "Concepts.h"
#include "Conversion.h"
#include "NumberTraits.h"
#include "Ordering.h"
#include "SSOVector.h"
#include "Types.h"
#include "cmath_compat.h"
namespace ARLib {
class BigInt {
    public:
    enum class Sign : bool { Plus, Minus };

    private:
    Sign m_sign = Sign::Plus;
    SSOVector<uint8_t, 20> m_buffer{};

    constexpr static inline uint8_t max_buffer_unit = 99;
    constexpr static inline uint8_t s_carry         = 1;
    constexpr static inline uint8_t max_sum_unit    = max_buffer_unit + max_buffer_unit + s_carry;
    static Ordering comparison_same_length(const BigInt& left, const BigInt& right);
    static BigInt multiplication(const BigInt& left, const BigInt& right);
    static BigInt division(const BigInt& left, const BigInt& right);
    static BigInt sign_agnostic_sum(const BigInt& left, const BigInt& right);
    static BigInt sign_agnostic_difference(const BigInt& left, const BigInt& right);
    static BigInt difference(const BigInt& left, const BigInt& right);
    static BigInt sum(const BigInt& left, const BigInt& right);
    void init_from_string(StringView view);
    void inplace_sum(const BigInt& other);
    void inplace_difference(const BigInt& other);
    void inplace_multiplication(const BigInt& other);
    void inplace_division(const BigInt& other);
    void trim_leading_zeros();
    void normalize_zero();
    static Ordering absolute_comparison(const BigInt& left, const BigInt& right);
    uint64_t to_absolute_value_for_division() const;
    void insert_back(uint8_t value);

    public:
    constexpr BigInt() = default;
    BigInt(Integral auto value) {
        if constexpr (IsSigned<decltype(value)>) {
            if (value < 0) {
                value  = -value;
                m_sign = Sign::Minus;
            }
        }

        while (value > 0) {
            uint8_t fragment = static_cast<uint8_t>(value % 100);
            value /= 100;
            m_buffer.append(fragment);
        }
    }
    // must be a string in decimal
    BigInt(const String& value);
    BigInt(StringView value);
    BigInt& operator++() {
        inplace_sum(BigInt{ 1 });
        return *this;
    }
    BigInt operator++(int) {
        BigInt copy = *this;
        inplace_sum(BigInt{ 1 });
        return copy;
    }
    BigInt& operator--() {
        inplace_difference(BigInt{ 1 });
        return *this;
    }
    BigInt operator--(int) {
        BigInt copy = *this;
        inplace_difference(BigInt{ 1 });
        return copy;
    }
    BigInt operator+(const BigInt& other) const { return sum(*this, other); }
    BigInt& operator+=(const BigInt& other) {
        inplace_sum(other);
        return *this;
    }
    BigInt operator-(const BigInt& other) const { return difference(*this, other); }
    BigInt& operator-=(const BigInt& other) {
        inplace_difference(other);
        return *this;
    }
    BigInt operator*(const BigInt& other) const { return multiplication(*this, other); }
    BigInt& operator*=(const BigInt& other) {
        inplace_multiplication(other);
        return *this;
    }
    BigInt operator/(const BigInt& other) const { return division(*this, other); }
    BigInt& operator/=(const BigInt& other) {
        inplace_division(other);
        return *this;
    }
    bool operator==(Integral auto other) const { return *this == BigInt{ other }; }
    bool operator==(const BigInt& other) const {
        if (this == &other) return true;
        if (m_buffer.size() != other.m_buffer.size()) return false;
        return comparison_same_length(*this, other) == equal;
    }
    bool operator!=(const BigInt& other) const { return !(*this == other); }
    bool operator!=(Integral auto other) const { return *this != BigInt{ other }; }
    bool operator>(const BigInt& other) const {
        if (this == &other) return false;
        size_t my_size    = size();
        size_t other_size = other.size();
        if (m_sign == other.m_sign) {
            if (my_size == other_size) {
                return comparison_same_length(*this, other) == greater;
            } else {
                return m_sign == Sign::Plus ? my_size > other_size : my_size < other_size;
            }
        } else {
            // if sign of this is positive, means other is negative, and that this is bigger than other
            return m_sign == Sign::Plus;
        }
    }
    bool operator>(Integral auto other) const { return *this > BigInt{ other }; }
    bool operator<(const BigInt& other) const {
        if (this == &other) return false;
        size_t my_size    = size();
        size_t other_size = other.size();
        if (m_sign == other.m_sign) {
            if (my_size == other_size) {
                return comparison_same_length(*this, other) == less;
            } else {
                return m_sign == Sign::Plus ? my_size < other_size : my_size > other_size;
            }
        } else {
            // if sign of this is negative, means other is positive, and that this is smaller than other
            return m_sign == Sign::Minus;
        }
    }
    bool operator<(Integral auto other) const { return *this < BigInt{ other }; }
    bool operator>=(const BigInt& other) const {
        const BigInt& th = *this;
        return (th > other) || (th == other);
    }
    bool operator>=(Integral auto other) const { return *this >= BigInt{ other }; }
    bool operator<=(const BigInt& other) const {
        const BigInt& th = *this;
        return (th < other) || (th == other);
    }
    bool operator<=(Integral auto other) const { return *this <= BigInt{ other }; }
    BigInt operator+() { return *this; }
    BigInt operator-() {
        BigInt copy = *this;
        copy.invert();
        return copy;
    }
    const SSOVector<uint8_t, 20>& buffer() const { return m_buffer; }
    Sign sign() const { return m_sign; }
    void invert() { m_sign = to_enum<Sign>(!from_enum(m_sign)); }
    size_t size() const { return m_buffer.size(); }
    bool fits() const;
    void clear() { m_buffer.clear_maintaning_capacity(); }
};
static inline const BigInt __bigint_zero = BigInt{};
static inline const BigInt __bigint_one  = BigInt{ 1 };
template <>
struct PrintInfo<BigInt> {
    const BigInt& m_value;
    PrintInfo(const BigInt& value) : m_value(value) {}
    String repr() const;
};
}    // namespace ARLib

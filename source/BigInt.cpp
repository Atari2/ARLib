#include "BigInt.hpp"
namespace ARLib {
DiscardResult<> BigInt::init_from_string(StringView value) {
    if (value.is_empty()) return {};
    m_buffer.reserve(value.size() / 2);
    if (value[0] == '-') { m_sign = Sign::Minus; }
    auto view = value.substringview(from_enum(m_sign));
    for (size_t i = view.size();; i -= 2) {
        auto subview = view.substringview((i - 1) == 0 ? i - 1 : i - 2, i);
        if (auto err = StrViewToInt(subview); err.is_error()) {
            return err.to_error();
        } else {
            m_buffer.append(static_cast<uint8_t>(err.to_ok()));
        }
        if (i <= 2) break;
    }
    return {};
}
BigInt::BigInt(const String& value) {
    init_from_string(value.view());
}
BigInt::BigInt(StringView value) {
    init_from_string(value);
}
// worst case O(N), best case O(1)
Ordering BigInt::comparison_same_length(const BigInt& left, const BigInt& right) {
    HARD_ASSERT(left.size() == right.size(), "This function assumes same size for the 2 BigInts")
    if (left.size() == 0) return equal;
    for (size_t i = left.size() - 1;; i--) {
        if (left.m_buffer[i] != right.m_buffer[i]) {
            if (left.m_buffer[i] < right.m_buffer[i]) {
                return less;
            } else {
                return greater;
            }
        }
        if (i == 0) break;
    }
    return equal;
}
// worst case O(N^2 * M) where N is the longer_buffer.size() and M is smaller_buffer.size()
BigInt BigInt::multiplication(const BigInt& left, const BigInt& right) {
    BigInt total{};
    for (size_t i = 0; i < left.size(); i++) {
        BigInt partial_result{};
        auto limb = left.m_buffer[i];
        for (size_t count = 0; count < i; count++) partial_result.m_buffer.append(00);
        uint8_t carryover = 0;
        for (auto mult : right.buffer()) {
            uint16_t partial = static_cast<uint16_t>(limb * mult + carryover);
            carryover        = static_cast<uint8_t>(partial / 100);
            partial_result.m_buffer.append(static_cast<uint8_t>(partial % 100));
        }
        if (carryover > 0) partial_result.m_buffer.append(carryover);
        total += partial_result;
    }
    if (left.sign() != right.sign()) total.m_sign = Sign::Minus;
    return total;
}
// checks if value fits into a uint64_t, without sign
bool BigInt::fits() const {
    return size() <= NumberTraits<uint64_t>::size;
}
uint64_t BigInt::to_absolute_value_for_division() const {
    uint64_t pows[] = { 1ull,
                        1'00ull,
                        1'00'00ull,
                        1'00'00'00ull,
                        1'00'00'00'00ull,
                        1'00'00'00'00'00ull,
                        1'00'00'00'00'00'00ull,
                        1'00'00'00'00'00'00'00ull };
    static_assert(sizeof_array(pows) == NumberTraits<uint64_t>::size);
    if (!fits()) return NumberTraits<uint64_t>::max;
    uint64_t value = 0;
    for (size_t i = 0; i < m_buffer.size(); i++) { value += (static_cast<uint64_t>(m_buffer[i]) * pows[i]); }
    return value;
}
uint64_t BigInt::to_absolute_value() const {
    if (sign() == Sign::Minus) {
        constexpr int64_t pows[] = { 1ll,
                                     1'00ll,
                                     1'00'00ll,
                                     1'00'00'00ll,
                                     1'00'00'00'00ll,
                                     1'00'00'00'00'00ll,
                                     1'00'00'00'00'00'00ll,
                                     1'00'00'00'00'00'00'00ll };
        static_assert(sizeof_array(pows) == NumberTraits<int64_t>::size);
        if (!fits()) return static_cast<uint64_t>(NumberTraits<int64_t>::min);
        int64_t value = 0;
        for (size_t i = 0; i < m_buffer.size(); i++) { value += (static_cast<int64_t>(m_buffer[i]) * pows[i]); }
        value = -value;
        return static_cast<uint64_t>(value);
    } else {
        constexpr uint64_t pows[] = { 1ull,
                                      1'00ull,
                                      1'00'00ull,
                                      1'00'00'00ull,
                                      1'00'00'00'00ull,
                                      1'00'00'00'00'00ull,
                                      1'00'00'00'00'00'00ull,
                                      1'00'00'00'00'00'00'00ull };
        if (!fits()) return NumberTraits<uint64_t>::max;
        uint64_t value = 0;
        for (size_t i = 0; i < m_buffer.size(); i++) { value += (static_cast<uint64_t>(m_buffer[i]) * pows[i]); }
        return value;
    }
}
// this is omega-slow in the case of a bigint with more than 16 digits
// divided by another bigint with more than 16 digits
// because it doesn't hit any fast path and it has to do a subtraction on a loop in a loop
// worst case complexity (with N = dividend.size()) : O(N * (N + 99*(N*N))) -> O(N^2 + 99N^3)
// but it works
BigInt BigInt::division(const BigInt& dividend, const BigInt& divisor) {
    bool dividend_zero = dividend == __bigint_zero;
    bool divisor_zero  = divisor == __bigint_zero;
    if (dividend_zero && divisor_zero) {
        ASSERT_NOT_REACHED("0 divided by 0 is not a defined operation");
    } else if (dividend_zero) {
        return __bigint_zero;
    } else if (divisor_zero) {
        ASSERT_NOT_REACHED("N divided by 0 is not a defined operation");
    }
    auto comp = absolute_comparison(dividend, divisor);
    if (comp == equal) {
        if (dividend.sign() == divisor.sign())
            return __bigint_one;
        else
            return BigInt{ -1 };
    } else if (comp == less) {
        return __bigint_zero;
    } else {
        if (absolute_comparison(divisor, __bigint_one) == equal) {
            BigInt result = dividend;
            result.m_sign = to_enum<Sign>(!(dividend.sign() == divisor.sign()));
            return result;
        } else if (divisor.fits() && dividend.fits()) {
            auto result =
            BigInt{ dividend.to_absolute_value_for_division() / divisor.to_absolute_value_for_division() };
            result.m_sign = to_enum<Sign>(!(dividend.sign() == divisor.sign()));
            return result;
        }

        BigInt result{};
        BigInt partial{};
        bool divisor_fits = divisor.fits();
        for (size_t i = dividend.size() - 1;; i--) {
            partial.insert_back(dividend.m_buffer[i]);
            if (partial > divisor) {
                BigInt subtraend{ divisor };
                uint8_t times = 1;
                if (partial.fits() && divisor_fits) {
                    times = static_cast<uint8_t>(
                    partial.to_absolute_value_for_division() / divisor.to_absolute_value_for_division()
                    );
                    subtraend *= times;
                } else {
                    auto sub_result = partial - subtraend;
                    while (sub_result > divisor) {
                        subtraend += divisor;
                        sub_result = partial - subtraend;
                        times++;
                    }
                }
                partial -= subtraend;
                result.insert_back(times);
            } else {
                result.insert_back(0x00);
            }
            if (i == 0) break;
        }
        result.trim_leading_zeros();
        result.m_sign = to_enum<Sign>(!(dividend.sign() == divisor.sign()));
        return result;
    }
}
void BigInt::insert_back(uint8_t value) {
    m_buffer.prepend(move(value));
}
void BigInt::inplace_multiplication(const BigInt& other) {
    // I don't immediately see a way to do multiplcation in-place
    // this is the easiest way otherwise
    auto result = multiplication(*this, other);
    m_sign      = result.sign();
    m_buffer    = move(result.m_buffer);
}
void BigInt::inplace_division(const BigInt& other) {
    // same as multiplication
    auto result = division(*this, other);
    m_sign      = result.sign();
    m_buffer    = move(result.m_buffer);
}
// worst case: O(N) where N is longer_buffer.size()
BigInt BigInt::sign_agnostic_sum(const BigInt& left, const BigInt& right) {
    BigInt result{};
    auto min_len               = min_bt(left.size(), right.size());
    auto max_len               = max_bt(left.size(), right.size());
    bool carry                 = false;
    const auto& left_buffer    = left.buffer();
    const auto& right_buffer   = right.buffer();
    const auto& longest_buffer = left.size() > right.size() ? left.buffer() : right.buffer();
    for (size_t i = 0; i < min_len; i++) {
        uint8_t sum_unit_result = static_cast<uint8_t>(left_buffer[i] + right_buffer[i] + carry);
        if (sum_unit_result > max_buffer_unit) {
            carry = true;
            sum_unit_result %= 100;
        } else {
            carry = false;
        }
        result.m_buffer.append(sum_unit_result);
    }
    for (size_t i = min_len; i < max_len; i++) {
        uint8_t sum_unit_result = longest_buffer[i] + carry;
        if (sum_unit_result > max_buffer_unit) {
            carry = true;
            sum_unit_result %= 100;
        } else {
            carry = false;
        }
        result.m_buffer.append(sum_unit_result);
    }
    if (carry) result.m_buffer.append(carry);
    return result;
}
// worst case O(N) where N is longer_buffer.size()
BigInt BigInt::sign_agnostic_difference(const BigInt& left, const BigInt& right) {
    BigInt result{};
    auto left_bigger       = absolute_comparison(left, right) == greater;
    const auto& max_val    = left_bigger ? left : right;
    const auto& min_val    = left_bigger ? right : left;
    auto max_len           = max_val.size();
    auto min_len           = min_val.size();
    bool borrow            = false;
    const auto& max_buffer = max_val.buffer();
    const auto& min_buffer = min_val.buffer();
    for (size_t i = 0; i < min_len; i++) {
        uint8_t left_limb  = max_buffer[i];
        bool double_borrow = false;
        if (left_limb == 0x00) {
            if (borrow) {
                left_limb     = 99;
                double_borrow = true;
            }
        } else {
            left_limb -= borrow;
        }
        uint8_t right_limb = min_buffer[i];
        if (left_limb >= right_limb) {
            borrow = double_borrow;
            result.m_buffer.append(left_limb - right_limb);
        } else {
            borrow = true;
            result.m_buffer.append(static_cast<uint8_t>((left_limb + 100) - right_limb));
        }
    }
    for (size_t i = min_len; i < max_len; i++) {
        result.m_buffer.append(max_buffer[i] - borrow);
        borrow = false;
    }
    if (left_bigger)
        result.m_sign = Sign::Plus;
    else
        result.m_sign = Sign::Minus;
    return result;
}
BigInt BigInt::difference(const BigInt& left, const BigInt& right) {
    if (left.sign() == right.sign()) {
        auto result = sign_agnostic_difference(left, right);
        if (left.sign() == Sign::Minus) { result.invert(); }
        result.normalize_zero();
        return result;
    } else {
        auto result = sign_agnostic_sum(left, right);
        if (left.sign() == Sign::Minus) { result.invert(); }
        result.normalize_zero();
        return result;
    }
}
BigInt BigInt::sum(const BigInt& left, const BigInt& right) {
    // max value of any byte in the buffer is 99
    // which means that the max value of a sum of 2 buffer units is 198 + 1 (for carry)
    // which fits in uint8_t, guaranteed no overflow
    if (left.sign() == right.sign()) {
        auto result   = sign_agnostic_sum(left, right);
        result.m_sign = left.sign();
        result.normalize_zero();
        return result;
    } else {
        auto result = sign_agnostic_difference(left, right);
        result.normalize_zero();
        return result;
    }
}
void BigInt::inplace_difference(const BigInt& other) {
    if (sign() == other.sign()) {
        const bool this_bigger = absolute_comparison(*this, other) == greater;
        auto min_len           = this_bigger ? other.size() : size();
        auto max_len           = this_bigger ? size() : other.size();
        bool borrow            = false;
        for (size_t i = 0; i < min_len; i++) {
            bool double_borrow = false;
            uint8_t left_limb  = (this_bigger ? m_buffer[i] : other.m_buffer[i]);
            if (left_limb == 0x00) {
                if (borrow) {
                    left_limb     = 99;
                    double_borrow = true;
                }
            } else {
                left_limb -= borrow;
            }
            uint8_t right_limb = this_bigger ? other.m_buffer[i] : m_buffer[i];
            if (left_limb >= right_limb) {
                borrow      = double_borrow;
                m_buffer[i] = left_limb - right_limb;
            } else {
                borrow      = true;
                m_buffer[i] = static_cast<uint8_t>(left_limb + 100 - right_limb);
            }
        }
        if (this_bigger) {
            m_buffer[min_len] -= borrow;
        } else {
            for (size_t i = min_len; i < max_len; i++) {
                m_buffer.append(other.m_buffer[i] - borrow);
                borrow = false;
            }
        }
        trim_leading_zeros();
        if (!this_bigger) invert();
    } else {
        Sign before_sign = m_sign;
        invert();
        inplace_sum(other);
        m_sign = before_sign;
    }
    trim_leading_zeros();
}
void BigInt::trim_leading_zeros() {
    while (m_buffer.last() == 0) {
        m_buffer.pop();
        if (m_buffer.size() == 0) break;
    }
}
void BigInt::normalize_zero() {
    trim_leading_zeros();
    if (m_sign == Sign::Plus) return;
    if (*this == __bigint_zero) m_sign = Sign::Plus;
}
Ordering BigInt::absolute_comparison(const BigInt& left, const BigInt& right) {
    if (&left == &right) return equal;
    if (left.size() > right.size()) {
        return greater;
    } else if (left.size() < right.size()) {
        return less;
    } else {
        for (size_t i = left.size() - 1;; i--) {
            if (left.m_buffer[i] > right.m_buffer[i])
                return greater;
            else if (left.m_buffer[i] < right.m_buffer[i])
                return less;
            if (i == 0) break;
        }
        return equal;
    }
}
void BigInt::inplace_sum(const BigInt& other) {
    if (sign() == other.sign()) {
        auto min_len            = min_bt(size(), other.size());
        auto max_len            = max_bt(size(), other.size());
        bool carry              = false;
        bool this_longer_buffer = max_len == size();
        for (size_t i = 0; i < min_len; i++) {
            uint8_t sum_unit_result = static_cast<uint8_t>(m_buffer[i] + other.m_buffer[i] + carry);
            if (sum_unit_result > max_buffer_unit) {
                carry = true;
                sum_unit_result %= 100;
            } else {
                carry = false;
            }
            m_buffer[i] = sum_unit_result;
        }
        for (size_t i = min_len; i < max_len; i++) {
            if (this_longer_buffer) {
                uint8_t sum_unit_result = m_buffer[i] + carry;
                if (sum_unit_result > max_buffer_unit) {
                    carry = true;
                    sum_unit_result %= 100;
                } else {
                    carry = false;
                }
                m_buffer[i] = sum_unit_result;
            } else {
                uint8_t sum_unit_result = other.m_buffer[i] + carry;
                if (sum_unit_result > max_buffer_unit) {
                    carry = true;
                    sum_unit_result %= 100;
                } else {
                    carry = false;
                }
                m_buffer.append(sum_unit_result);
            }
        }
        if (carry) m_buffer.append(carry);
    } else {
        invert();
        Sign sign = m_sign;
        inplace_difference(other);
        if (sign == m_sign) invert();
    }
}
String PrintInfo<BigInt>::repr() const {
    const auto& buffer = m_value.buffer();
    if (buffer.size() == 0) { return "0"_s; }
    String num{};
    if (m_value.sign() == BigInt::Sign::Minus) num.append('-');
    size_t sz = buffer.size() - 1;

    for (size_t i = sz;; i--) {
        if (i == sz) {
            num.append(IntToStr(buffer[i]));
        } else {
            auto str = IntToStr(buffer[i]);
            if (str.size() == 1) num.append('0');
            num.append(move(str));
        }
        if (i == 0) break;
    }
    return num;
}
}    // namespace ARLib

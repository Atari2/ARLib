#include "BigInt.h"
#include "Printer.h"

namespace ARLib {

    BigInt::BigInt(const String& value) {
        if (value.is_empty()) return;
        m_buffer.reserve(value.size() / 2);
        if (value[0] == '-') { m_sign = Sign::Minus; }
        auto view = value.substringview(from_enum(m_sign));
        for (size_t i = view.size();; i -= 2) {
            auto subview = view.substringview((i - 1) == 0 ? i - 1 : i - 2, i);
            m_buffer.append(static_cast<uint8_t>(StrViewToInt(subview)));
            if (i <= 2) break;
        }
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
            for (size_t count = 0; count < i; count++)
                partial_result.m_buffer.append(00);
            uint8_t carryover = 0;
            for (auto mult : right.buffer()) {
                uint16_t partial = limb * mult + carryover;
                carryover = static_cast<uint8_t>(partial / 100);
                partial_result.m_buffer.append(static_cast<uint8_t>(partial % 100));
            }
            if (carryover > 0) partial_result.m_buffer.append(carryover);
            total += partial_result;
        }
        if (left.sign() != right.sign()) total.m_sign = Sign::Minus;
        return total;
    }

    BigInt BigInt::division(const BigInt& dividend, const BigInt& divisor) {
        bool dividend_zero = dividend == __bigint_zero;
        bool divisor_zero = divisor == __bigint_zero;
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
                return BigInt{1};
            else
                return BigInt{-1};
        } else if (comp == less) {
            return __bigint_zero;
        } else {
            if (absolute_comparison(divisor, BigInt{1}) == equal) {
                BigInt result = dividend;
                result.m_sign = to_enum<Sign>(!(dividend.sign() == divisor.sign()));
                return result;
            }
            // TODO: normal algorithm
            // remove placeholder when done
            return __bigint_zero;
        }
    }

    void BigInt::inplace_multiplication(const BigInt& other) {
        // I don't immediately see a way to do multiplcation in-place
        // this is the easiest way otherwise
        auto result = multiplication(*this, other);
        m_sign = result.sign();
        m_buffer = move(result.m_buffer);
    }

    void BigInt::inplace_division(const BigInt& other) {}

    // worst case: O(N) where N is longer_buffer.size()
    BigInt BigInt::sign_agnostic_sum(const BigInt& left, const BigInt& right) {
        BigInt result{};
        auto min_len = min_bt(left.size(), right.size());
        auto max_len = max_bt(left.size(), right.size());
        bool carry = false;
        const auto& left_buffer = left.buffer();
        const auto& right_buffer = right.buffer();
        const auto& longest_buffer = left.size() > right.size() ? left.buffer() : right.buffer();
        for (size_t i = 0; i < min_len; i++) {
            uint8_t sum_unit_result = left_buffer[i] + right_buffer[i] + carry;
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
        auto left_bigger = absolute_comparison(left, right) == greater;
        const auto& max_val = left_bigger ? left : right;
        const auto& min_val = left_bigger ? right : left;
        auto max_len = max_val.size();
        auto min_len = min_val.size();
        bool borrow = false;
        const auto& max_buffer = max_val.buffer();
        const auto& min_buffer = min_val.buffer();
        bool has_swapped_operands = max_val != left;
        for (size_t i = 0; i < min_len; i++) {
            uint8_t left_limb = max_buffer[i] - borrow;
            uint8_t right_limb = min_buffer[i];
            if (left_limb >= right_limb) {
                borrow = false;
                result.m_buffer.append(left_limb - right_limb);
            } else {
                borrow = true;
                result.m_buffer.append((left_limb + 100) - right_limb);
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
            auto result = sign_agnostic_sum(left, right);
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
            auto min_len = this_bigger ? other.size() : size();
            auto max_len = this_bigger ? size() : other.size();
            bool borrow = false;
            for (size_t i = 0; i < min_len; i++) {
                uint8_t left_limb = (this_bigger ? m_buffer[i] : other.m_buffer[i]) - borrow;
                uint8_t right_limb = this_bigger ? other.m_buffer[i] : m_buffer[i];
                if (left_limb >= right_limb) {
                    borrow = false;
                    m_buffer[i] = left_limb - right_limb;
                } else {
                    borrow = true;
                    m_buffer[i] = left_limb + 100 - right_limb;
                }
            }
            if (this_bigger) {
                m_buffer[min_len] -= borrow;
            } else {
                for (size_t i = min_len; i <= max_len; i++) {
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
            auto min_len = min_bt(size(), other.size());
            auto max_len = max_bt(size(), other.size());
            bool carry = false;
            bool this_longer_buffer = max_len == size();
            for (size_t i = 0; i < min_len; i++) {
                uint8_t sum_unit_result = m_buffer[i] + other.m_buffer[i] + carry;
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

} // namespace ARLib
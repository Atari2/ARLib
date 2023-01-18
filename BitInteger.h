#include "CharConv.h"
#include "Concepts.h"
#include "PrintInfo.h"
#include "Tuple.h"
#include "TypeTraits.h"
#include "Utility.h"
namespace ARLib {
constexpr size_t CountOneBits(Integral auto value) {
    using T         = RemoveCvRefT<decltype(value)>;
    size_t one_bits = 0;
    constexpr T zero{ 0 };
    while (value != zero) {
        one_bits += (value & 1);
        value >>= 1;
    }
    return one_bits;
}

template <size_t Width>
class BitInteger;
template <size_t Width>
class BitInteger {
    static_assert(Width > 0 && Width <= 64, "Bit width needs to be between 0 and 64");
    using T = ConditionalT<
    (Width > 8), ConditionalT<(Width > 16), ConditionalT<(Width > 32), uint64_t, uint32_t>, uint16_t>, uint8_t>;

    using Make = BitInteger<Width>;

    template <typename Conv>
    constexpr static inline bool BitIntegerConvertible = Integral<Conv> || SameAs<Conv, BitInteger<Width>>;

    T m_value{ 0 };

    constexpr static size_t ContainingBits = sizeof(T) * BITS_PER_BYTE;
    constexpr static size_t BitMask        = (1_sz << Width) - 1_sz;
    constexpr T clamp() const { return m_value & BitMask; }
    constexpr T cast(Integral auto value) const { return static_cast<T>(value) & BitMask; }
    constexpr auto& assign_ret(Integral auto value) {
        m_value = cast(value);
        return *this;
    }
    template <typename Val>
    requires BitIntegerConvertible<Val>
    constexpr static T get_value(const Val& value) {
        if constexpr (SameAs<Val, BitInteger<Width>>) {
            return value.m_value;
        } else {
            return static_cast<T>(value);
        }
    }

    public:
    constexpr BitInteger(Integral auto value) : m_value(cast(value)) {}
    constexpr T value() const { return clamp(); }
    constexpr operator T() const { return value(); }
    constexpr size_t width() const { return Width; }
    constexpr auto operator++(int) {
        auto value = *this;
        m_value    = cast(m_value + 1);
        return value;
    }
    constexpr auto operator--(int) {
        auto value = *this;
        m_value    = cast(m_value - 1);
        return value;
    }
    constexpr auto& operator++() {
        m_value = cast(m_value + 1);
        return *this;
    }
    constexpr auto& operator--() {
        m_value = cast(m_value - 1);
        return *this;
    }
    constexpr auto& operator=(const BitInteger<Width>& value) { return assign_ret(value.m_value); }
    constexpr auto operator~() const { return Make{ ~m_value }; }
#define DEFINE_OPERATOR(op)                                                                                            \
    template <typename Ty>                                                                                             \
    requires BitIntegerConvertible<Ty>                                                                                 \
    constexpr auto operator op(const Ty& value) const {                                                                \
        return Make{ m_value op get_value(value) };                                                                    \
    }                                                                                                                  \
    template <typename Ty>                                                                                             \
    requires BitIntegerConvertible<Ty>                                                                                 \
    constexpr auto& operator op##=(const Ty& value) const {                                                            \
        return assign_ret(m_value op get_value(value));                                                                \
    }

    DEFINE_OPERATOR(*)
    DEFINE_OPERATOR(/)
    DEFINE_OPERATOR(%)
    DEFINE_OPERATOR(+)
    DEFINE_OPERATOR(-)
    DEFINE_OPERATOR(<<)
    DEFINE_OPERATOR(>>)
    DEFINE_OPERATOR(&)
    DEFINE_OPERATOR(^)
    DEFINE_OPERATOR(|)

#undef DEFINE_OPERATOR
};
template <size_t Width>
struct PrintInfo<BitInteger<Width>> {
    const BitInteger<Width>& m_value;
    explicit PrintInfo(const BitInteger<Width>& value) : m_value(value) {}
    String repr() {
        constexpr size_t div = ((Width / 8) + ((Width % 8) != 0)) * 2;
        return String::formatted("%0*X", div, m_value.value());
    }
};
using uint24_t = BitInteger<24>;
using uint40_t = BitInteger<40>;
using uint48_t = BitInteger<48>;
using uint56_t = BitInteger<56>;
constexpr uint24_t operator""_u24(unsigned long long val) {
    return uint24_t{ val };
}
constexpr uint40_t operator""_u40(unsigned long long val) {
    return uint40_t{ val };
}
constexpr uint48_t operator""_u48(unsigned long long val) {
    return uint48_t{ val };
}
constexpr uint56_t operator""_u56(unsigned long long val) {
    return uint56_t{ val };
}
template <size_t Width>
using uintn_t = BitInteger<Width>;
}    // namespace ARLib
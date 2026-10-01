#include "DoubleFromChars.hpp"
namespace ARLib {
// FIXME: a lot of issues
//        does not handle E e exponents
//        does not handle > 19 digits for double precision numbers
//        does not handle subnormals
//        does not handle inf, nan, etc...

Result<double, DoubleFromCharsError> DoubleFromChars(const char* str, size_t size) {
    if (str == nullptr || size == 0) return DoubleFromCharsError::InvalidArgument;
    double value          = 0.0;
    const char* ptr       = str;
    const char* const end = str + size;
    bool neg              = false;
    while (isspace(*ptr) && ptr < end) { ptr++; }
    if (ptr == end) return DoubleFromCharsError::InvalidArgument;
    if (*ptr == '+') {
        ptr++;
    } else if (*ptr == '-') {
        ptr++;
        neg = true;
    }

    if (ptr == end) return DoubleFromCharsError::InvalidArgument;
    size_t ndigits      = 0;
    size_t leadingzeros = 0;
    const char* dotpos  = nullptr;

    uint64_t acc = 0;

    constexpr size_t dof = 19;

    while (true) {
        if (ptr == end) break;
        if (isdigit(*ptr)) {
            acc = (10 * acc) + static_cast<uint64_t>(*ptr - '0');
            if (acc != 0) {
                ndigits++;
            } else {
                leadingzeros++;
            }
        } else if (*ptr == '.') {
            dotpos = ptr;
        } else {
            return DoubleFromCharsError::InvalidArgument;
        }
        ++ptr;
    }

    if (ndigits == 0) return DoubleFromCharsError::InvalidArgument;
    if (dotpos == nullptr) {
        // integer case
        if (ndigits > dof) {
            // slow case, high precision
            TODO("slow case > 19 digits for integer doublefromchars");
        } else {
            return static_cast<double>(acc) * (neg ? -1.0 : 1.0);
        }
    } else {
        int64_t exponent = static_cast<int64_t>(dotpos - str) - static_cast<int64_t>(ndigits + leadingzeros);
        double pow10 = pow(10.0, static_cast<double>(exponent));
        if (ndigits > dof) {
            // slow case, high precision
            TODO("slow case > 19 digits for doublefromchars");
        } else {
            // fast case
            return static_cast<double>(acc) * pow10 * (neg ? -1.0 : 1.0);
        }
    }
    return value;
}
}    // namespace ARLib
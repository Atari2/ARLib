#include "CharConv.h"
#include "StringView.h"
#include "Vector.h"
#include <charconv>

namespace ARLib {

    // FIXME: Write a routine that actually works
    // using the stdlib for now because this is too hard to get right
    // one day I'll actual write it properly
    double StrViewToDouble(const StringView str) {
        double val = 0.0;
        auto res = std::from_chars(str.data(), str.data() + str.size(), val);
        HARD_ASSERT(res.ec != std::errc::invalid_argument && res.ec != std::errc::result_out_of_range,
                    "Failed to convert string to double")
        return val;
    }
    float StrViewToFloat(const StringView str) {
        return static_cast<float>(StrViewToDouble(str));
    }
    double StrToDouble(const String& str) {
        return StrViewToDouble(str.view());
    }
    float StrToFloat(const String& str) {
        return StrViewToFloat(str.view());
    }

    String DoubleToStrImpl(double value, const char* fmt) {
#ifdef WINDOWS
        const auto len = static_cast<size_t>(scprintf(fmt, value));
        String str{};
        str.reserve(len);
        sprintf(str.rawptr(), fmt, value);
        str.set_size(len);
        return str;
#else
        const int n = 308 /* numeric limits length for dbl */ + 20;
        String str{};
        str.reserve(n);
        int written = snprintf(str.rawptr(), n, fmt, value);
        HARD_ASSERT((written > 0), "Failed to write double to string");
        str.set_size(static_cast<size_t>(written));
        return str;
#endif
    }

    String LongDoubleToStrImpl(long double value, const char* fmt) {
#ifdef WINDOWS
        return DoubleToStrImpl(value, fmt);
#else
        const int n = 4932 /* numeric limits length for dbl */ + 20;
        String str{};
        str.reserve(n);
        int written = snprintf(str.rawptr(), n, fmt, value);
        HARD_ASSERT((written > 0), "Failed to write long double to string");
        str.set_size(static_cast<size_t>(written));
        return str;
#endif
    }

    String FloatToStrImpl(float value, const char* fmt) {
#ifdef WINDOWS
        return DoubleToStrImpl(static_cast<double>(value), fmt);
#else
        const int n = 38 /* numeric limits length for flt */ + 20;
        String str{};
        str.reserve(38);
        int written = snprintf(str.rawptr(), n, fmt, static_cast<double>(value));
        HARD_ASSERT((written > 0), "Failed to write float to string");
        str.set_size(static_cast<size_t>(written));
        return str;
#endif
    }

} // namespace ARLib
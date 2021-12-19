#include "CharConv.h"
#include "Vector.h"

namespace ARLib {

    // FIXME: make this more efficient
    double StrToDouble(const String& str) {
        auto parts = str.split_view_at_any(".,");
        auto len = parts.size();
        if (len == 1) return static_cast<double>(StrToI64(str));
        if (len > 2) return NumericLimits::NanD;
        auto integral_part = static_cast<double>(StrViewToI64(parts[0]));
        double sign = 1.0;
        if (integral_part < 0) sign = -1.0;
        integral_part = abs(integral_part);
        auto fract_part = static_cast<double>(StrViewToI64(parts[1]));
        fract_part /= pow(10.0, static_cast<double>(parts[1].length()));
        return (integral_part + fract_part) * sign;
    }

    float StrToFloat(const String& str) { return static_cast<float>(StrToDouble(str)); }

    String DoubleToStr(double value) {
#ifdef WINDOWS
        const auto len = static_cast<size_t>(scprintf("%f", value));
        String str{};
        str.reserve(len);
        sprintf(str.rawptr(), "%f", value);
        str.set_size(len);
        return str;
#else
        const int n = 308 /* numeric limits length for dbl */ + 20;
        String str{};
        str.reserve(n);
        int written = snprintf(str.rawptr(), n, "%f", value);
        HARD_ASSERT((written > 0), "Failed to write double to string");
        str.set_size(static_cast<size_t>(written));
        return str;
#endif
    }

    String LongDoubleToStr(long double value) {
#ifdef WINDOWS
        return DoubleToStr(value);
#else
        const int n = 4932 /* numeric limits length for dbl */ + 20;
        String str{};
        str.reserve(n);
        int written = snprintf(str.rawptr(), n, "%Lf", value);
        HARD_ASSERT((written > 0), "Failed to write long double to string");
        str.set_size(static_cast<size_t>(written));
        return str;
#endif
    }
    String FloatToStr(float value) {
#ifdef WINDOWS
        return DoubleToStr(static_cast<double>(value));
#else
        const int n = 38 /* numeric limits length for flt */ + 20;
        String str{};
        str.reserve(38);
        int written = snprintf(str.rawptr(), n, "%f", static_cast<double>(value));
        HARD_ASSERT((written > 0), "Failed to write float to string");
        str.set_size(static_cast<size_t>(written));
        return str;
#endif
    }

} // namespace ARLib
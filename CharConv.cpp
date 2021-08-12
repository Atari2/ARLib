#include "CharConv.h"

namespace ARLib {
    // FIXME: avoid pow/round calls in Str{View}To{I64/int}

    int64_t StrViewToI64(const StringView view, int base) {
        if (base < 2 || base > 36) return 0;

        size_t cur_index = 0;
        size_t max_index = view.length();
        // skip whitespace
        while (isspace(view[cur_index])) {
            cur_index++;
        }

        if (cur_index == max_index) return 0;

        int sign = 1;
        char s = view[cur_index];
        if (s == '+' || s == '-') {
            sign = s == '+' ? 1 : -1;
            cur_index++;
        }

        if (cur_index == max_index) return 0ll * sign;

        if (base == 16) {
            if (view[cur_index] == '0' && tolower(view[cur_index + 1]) == 'x') cur_index += 2;
        } else if (base == 2) {
            if (view[cur_index] == '0' && tolower(view[cur_index + 1]) == 'b') cur_index += 2;
        }

        // skip leading zeros
        while (view[cur_index] == '0') {
            cur_index++;
        }

        if (cur_index == max_index) return 0ll * sign;

        // 0-9 => 48-57
        // A-Z => 65-90

        int64_t total = 0;
        double pw = 0.0;
        for (size_t opp = max_index - 1; opp >= cur_index; opp--) {
            char c = toupper(view[opp]);
            if (!isalnum(c)) return total * sign;
            int num = c >= 'A' ? (c - 55) : (c - 48);
            if (num >= base) return total;
            total += static_cast<int64_t>(round((num * pow(static_cast<double>(base), pw))));
            pw += 1.0;
            if (opp == cur_index) break;
        }
        return total * sign;
    }

    int StrViewToInt(const StringView view, int base) { return static_cast<int>(StrViewToI64(view, base)); }

    int64_t StrToI64(const String& str, int base) {
        if (base < 2 || base > 36) return 0;

        size_t cur_index = 0;
        size_t max_index = str.length();
        // skip whitespace
        while (isspace(str[cur_index])) {
            cur_index++;
        }

        if (cur_index == max_index) return 0;

        int sign = 1;
        char s = str[cur_index];
        if (s == '+' || s == '-') {
            sign = s == '+' ? 1 : -1;
            cur_index++;
        }

        if (cur_index == max_index) return 0ll * sign;

        if (base == 16) {
            if (str[cur_index] == '0' && tolower(str[cur_index + 1]) == 'x') cur_index += 2;
        } else if (base == 2) {
            if (str[cur_index] == '0' && tolower(str[cur_index + 1]) == 'b') cur_index += 2;
        }

        // skip leading zeros
        while (str[cur_index] == '0') {
            cur_index++;
        }

        if (cur_index == max_index) return 0ll * sign;

        // 0-9 => 48-57
        // A-Z => 65-90

        int64_t total = 0;
        double pw = 0.0;
        for (size_t opp = max_index - 1; opp >= cur_index; opp--) {
            char c = toupper(str[opp]);
            if (!isalnum(c)) return total * sign;
            int num = c >= 'A' ? (c - 55) : (c - 48);
            if (num >= base) return total;
            total += static_cast<int64_t>(round((num * pow(static_cast<double>(base), pw))));
            pw += 1.0;
            if (opp == cur_index) break;
        }
        return total * sign;
    }

    int StrToInt(const String& str, int base) { return static_cast<int>(StrToI64(str, base)); }

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
        String str{len};
        sprintf(str.rawptr(), "%f", value);
        str.set_size(len);
        return str;
#else
        const int n = 308 /* numeric limits length for dbl */ + 20;
        String str{n};
        int written = snprintf(str.rawptr(), n, "%f", value);
        HARD_ASSERT((written > 0), "Failed to write double to string");
        str.set_size(static_cast<size_t>(written));
        return str;
#endif
    }
    String FloatToStr(float value) {
#ifdef WINDOWS
        return DoubleToStr(static_cast<double>(value));
#else
        const int n = 38 /* numeric limits length for flt */ + 20;
        String str{n};
        int written = snprintf(str.rawptr(), n, "%f", static_cast<double>(value));
        HARD_ASSERT((written > 0), "Failed to write float to string");
        str.set_size(static_cast<size_t>(written));
        return str;
#endif
    }
}
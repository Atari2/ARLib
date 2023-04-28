#include "arlib_osapi.hpp"

#include "StringView.hpp"
#include "WStringView.hpp"
#include "String.hpp"
#include "WString.hpp"
#include "cstdio_compat.hpp"
#include "String.hpp"
#include "Vector.hpp"
#include "Array.hpp"
#include <clocale>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
namespace ARLib {
struct SetupLocale {
    String currlocale{};
    SetupLocale() {
        currlocale = String{ std::setlocale(LC_CTYPE, nullptr) };
        if (currlocale.ends_with(".UTF-8"_sv) || currlocale.ends_with(".utf8"_sv)) return;    // already utf8
        String half      = currlocale.substring(0, currlocale.index_of('.'));
        String newlocale = half + ".UTF-8"_s;
        Array common_locales{
            "C.UTF-8"_sv, "C.utf8"_sv, "en_US.UTF-8"_sv, "en_US.utf8"_sv, "it_IT.UTF-8"_sv, "it_IT.utf8"_sv,
        };
        if (std::setlocale(LC_CTYPE, newlocale.data()) == nullptr) {
            // we failed with the utf8 locale being used, try other common locales
            for (auto locale : common_locales) {
                if (std::setlocale(LC_CTYPE, locale.data()) != nullptr) return;
            }
            // we failed with all common locales, exit
            ASSERT_NOT_REACHED_FMT(
            "Failed to set locale to a valid UTF-8 locale, original locale was %s", currlocale.data()
            );
        }
    }
    ~SetupLocale() {
        // restore locale on exit
        std::setlocale(LC_CTYPE, currlocale.data());
    }
};
static const SetupLocale __localeSet{};
void print_last_error() {
    auto error = *__errno_location();
    if (error != 0) { puts(strerror(error)); }
}
String last_error() {
    auto error = *__errno_location();
    if (error != 0) {
        return String{ strerror(error) };
    } else {
        return String{};
    }
}
WString string_to_wstring(StringView str) {
    WString wstr{};
    wstr.reserve(str.size());
    size_t i = 0;
    while (i < str.size()) {
        wchar_t c;
        int ret = mbtowc(&c, str.data() + i, str.size() - i);
        if (ret == -1) {
            // handle failure
            wstr.append(L'?');
            i++;
        } else {
            wstr.append(c);
            i += static_cast<size_t>(ret);
        }
    }
    return wstr;
}
String wstring_to_string(WStringView wstr) {
    char buf[32];
    String str{};
    str.reserve(wstr.size());
    for (wchar_t c : wstr) {
        int ret = wctomb(buf, c);
        if (ret == -1) {
            // handle failure
            str.append('?');
        }
        str.append(StringView{ buf, static_cast<size_t>(ret) });
    }
    return str;
}
}    // namespace ARLib

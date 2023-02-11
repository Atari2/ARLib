#include "../arlib_osapi.h"

#include "../StringView.h"
#include "../WStringView.h"
#include "../String.h"
#include "../WString.h"
#include "../cstdio_compat.h"
#include "../String.h"
#include "../Vector.h"
#include "../Array.h"
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
        String half      = currlocale.split(".")[0];
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
    size_t required_size = mbstowcs(NULL, str.data(), 0);
    if (required_size == static_cast<size_t>(-1)) { return {}; }
    WString wstr{};
    wstr.reserve(required_size);
    size_t conv = mbstowcs(wstr.rawptr(), str.data(), wstr.capacity());
    wstr.set_size(conv);
    return wstr;
}
String wstring_to_string(WStringView wstr) {
    size_t required_size = wcstombs(NULL, wstr.data(), 0);
    if (required_size == static_cast<size_t>(-1)) { return {}; }
    String str{};
    str.reserve(required_size);
    size_t conv = wcstombs(str.rawptr(), wstr.data(), str.capacity());
    str.set_size(conv);
    return str;
}
}    // namespace ARLib

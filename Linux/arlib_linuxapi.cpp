#include "../arlib_osapi.h"

#include "../StringView.h"
#include "../WStringView.h"
#include "../String.h"
#include "../WString.h"
#include "../cstdio_compat.h"

#include <errno.h>
#include <string.h>
#include <stdlib.h>
namespace ARLib {
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

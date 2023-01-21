#include "../arlib_osapi.h"

#include "../StringView.h"
#include "../WStringView.h"
#include "../String.h"
#include "../WString.h"
#include "../cstdio_compat.h"

#include <errno.h>
#include <string.h>
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
    return {};
}
String wstring_to_string(WStringView wstr) {
    return {};
}
}    // namespace ARLib

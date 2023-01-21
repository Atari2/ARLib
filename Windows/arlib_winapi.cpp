#include "../arlib_osapi.h"

#include "../StringView.h"
#include "../WStringView.h"
#include "../String.h"
#include "../WString.h"
#include "../cstdio_compat.h"
#include "win_native_structs.h"
#include <windows.h>
namespace ARLib {
void print_last_error() {
    auto last_error = GetLastError();
    if (last_error != 0) {
        LPSTR buffer = nullptr;
        FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, last_error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&buffer), 0, NULL
        );
        puts(buffer);
        LocalFree(buffer);
    }
}
String last_error() {
    auto last_error = GetLastError();
    if (last_error != 0) {
        LPSTR buffer = nullptr;
        FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, last_error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&buffer), 0, NULL
        );
        String message{ buffer };
        LocalFree(buffer);
        return message;
    } else {
        return String{};
    }
}
String WideToUTF8(const wchar_t* from, const int from_size) {
    int required_size = WideCharToMultiByte(CP_UTF8, 0, from, from_size, NULL, 0, NULL, NULL);
    if (required_size < 0) { return {}; }
    String str{};
    str.reserve(required_size);
    int size = WideCharToMultiByte(CP_UTF8, 0, from, from_size, str.rawptr(), str.capacity(), NULL, NULL);
    str.set_size(size);
    return str;
}
WString UTF8ToWide(const char* from, const int from_size) {
    int convertResult = MultiByteToWideChar(CP_UTF8, 0, from, from_size, NULL, 0);
    if (convertResult <= 0) {
        return {};
    } else {
        WString wstr{};
        wstr.reserve(static_cast<size_t>(convertResult + 1));
        int conv   = MultiByteToWideChar(CP_UTF8, 0, from, from_size, wstr.rawptr(), convertResult);
        wstr.set_size(conv);
        return wstr;
    }
}
WString string_to_wstring(StringView str) {
    return UTF8ToWide(str.data(), static_cast<int>(str.size()));
}
String wstring_to_string(WStringView wstr) {
    return WideToUTF8(wstr.data(), static_cast<int>(wstr.size()));
}
}    // namespace ARLib

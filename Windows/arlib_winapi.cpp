#include "../arlib_osapi.h"

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include "../String.h"
#include "../cstdio_compat.h"
#include "win_native_structs.h"
#include <windows.h>

namespace ARLib {

    void print_last_error() {
        auto last_error = GetLastError();
        if (last_error != 0) {
            LPSTR buffer = nullptr;
            FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                           NULL, last_error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                           reinterpret_cast<LPSTR>(&buffer), 0, NULL);
            puts(buffer);
            LocalFree(buffer);
        }
    }

    String last_error() {
        auto last_error = GetLastError();
        if (last_error != 0) {
            LPSTR buffer = nullptr;
            FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                           NULL, last_error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                           reinterpret_cast<LPSTR>(&buffer), 0, NULL);
            String message{buffer};
            LocalFree(buffer);
            return message;
        } else {
            return String{};
        }
    }

} // namespace ARLib
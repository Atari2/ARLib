#include "SourceLocation.h"
#include "Assertion.h"
#include "Compat.h"
#ifdef WINDOWS
    #include <Windows.h>
#endif
#include "CharConvHelpers.h"
namespace ARLib {
[[maybe_unused]] static size_t len_needed_for_source_loc(const SourceLocation& loc, const char* message) {
    size_t len = message == nullptr ? 0 : strlen(message);
    len += strlen(loc.file_name());
    len += strlen(loc.function_name());
    len += StrLenFromIntegral<10>(loc.line());
    len += strlen("%s(%d): in function '%s': ");
    return len + 1;
}
void __print_debug_source_location(const SourceLocation& loc_, [[maybe_unused]] const char* message) {
#ifdef DEBUG_NEW_DELETE
    _assert_printf("%s", message);
    _assert_printf(
    "Function `%s` in file %s, at line %u and column %u\n", loc_.function_name(), loc_.file_name(), loc_.line(),
    loc_.column()
    );
#else
    ARLib::printf("%s", message);
    ARLib::printf(
    "Function `%s` in file %s, at line %u and column %u\n", loc_.function_name(), loc_.file_name(), loc_.line(),
    loc_.column()
    );
#endif
#ifdef WINDOWS
    if (IsDebuggerPresent()) {
        const size_t len = len_needed_for_source_loc(loc_, message);
        char* buf        = new char[len];
        int ret          = 0;
        if (message != nullptr) {
            ret = ARLib::snprintf(
            buf, len, "%s(%d): in function '%s': %s", loc_.file_name(), loc_.line(), loc_.function_name(), message
            );
        } else {
            ret = ARLib::snprintf(
            buf, len, "%s(%d): in function '%s'\n", loc_.file_name(), loc_.line(), loc_.function_name()
            );
        }
        buf[ret] = '\0';
        OutputDebugStringA(buf);
        delete[] buf;
    }
#endif
}
}    // namespace ARLib
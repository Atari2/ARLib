#pragma once

namespace ARLib {
    // TODO: Finish this implementation
    enum class PrintfErrorCodes { Ok, InvalidFormat, MissingType };
    PrintfErrorCodes printf_impl(const char* fmt, ...);
}
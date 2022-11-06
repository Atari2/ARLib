#pragma once

namespace ARLib {
    enum class PrintfErrorCodes { Ok, InvalidFormat, MissingType };
    PrintfErrorCodes printf_impl(const char* fmt, ...);
}
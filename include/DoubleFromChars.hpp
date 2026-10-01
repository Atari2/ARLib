#pragma once
#include "Result.hpp"
#include "EnumHelpers.hpp"
namespace ARLib {
MAKE_FANCY_ENUM(DoubleFromCharsError, uint8_t, InvalidArgument, ResultOutOfRange, UnknownError);

Result<double, DoubleFromCharsError> DoubleFromChars(const char* str, size_t size);
}    // namespace ARLib
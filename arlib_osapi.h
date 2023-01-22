#pragma once

#include "Types.h"
namespace ARLib {
class StringView;
class WStringView;
class String;
class WString;

void print_last_error();
String last_error();
WString string_to_wstring(StringView str);
String wstring_to_string(WStringView wstr);
}    // namespace ARLib

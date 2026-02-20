#pragma once

#include "Types.hpp"
namespace ARLib {
class StringView;
class WStringView;
class String;
class WString;

String get_env(StringView name);
bool set_env(StringView name, StringView value);
bool has_env(StringView name);
bool unset_env(StringView name);
void print_last_error();
String last_error();
WString string_to_wstring(StringView str);
String wstring_to_string(WStringView wstr);
size_t get_page_size();
}    // namespace ARLib

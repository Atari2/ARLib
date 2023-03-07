#include "CharConv.h"
#include "Assertion.h"
#include "StringView.h"
#include "Vector.h"
#include <charconv>
namespace ARLib {
// FIXME: Write a routine that actually works
// using the stdlib for now because this is too hard to get right
// one day I'll actual write it properly
double StrViewToDouble(const StringView str) {
    double val                = 0.0;
    [[maybe_unused]] auto res = std::from_chars(str.data(), str.data() + str.size(), val);
    SOFT_ASSERT(
    res.ec != std::errc::invalid_argument && res.ec != std::errc::result_out_of_range,
    "Failed to convert string to double"
    )
    // FIXME: find a way to make this function comunicate errors to the caller, current we just ignore everything
    //        that fails and return 0.0
    return val;
}
float StrViewToFloat(const StringView str) {
    return static_cast<float>(StrViewToDouble(str));
}
double StrToDouble(const String& str) {
    return StrViewToDouble(str.view());
}
float StrToFloat(const String& str) {
    return StrViewToFloat(str.view());
}
String DoubleToStrImpl(double value, const char* fmt, int precision) {
    const auto len = static_cast<size_t>(scprintf(fmt, value)) + static_cast<size_t>(precision);
    String str{};
    str.reserve(len);
    char fmtc  = ARLib::tolower(fmt[1]);
    bool upper = fmtc != fmt[1];
    std::chars_format char_format[]{ std::chars_format::scientific, std::chars_format::fixed,
                                     std::chars_format::general };
    char* ptr    = str.rawptr();
    auto ec      = std::to_chars(ptr, ptr + len, value, char_format[fmtc - 'e'], precision);
    size_t wrlen = static_cast<size_t>(ec.ptr - str.rawptr());
    str.set_size(wrlen);
    return upper ? str.upper() : str;
}
String LongDoubleToStrImpl(long double value, const char* fmt, int precision) {
    const auto len = static_cast<size_t>(scprintf(fmt, value)) + static_cast<size_t>(precision);
    String str{};
    str.reserve(len);
    char fmtc  = ARLib::tolower(fmt[2]);
    bool upper = fmtc != fmt[2];
    std::chars_format char_format[]{ std::chars_format::scientific, std::chars_format::fixed,
                                     std::chars_format::general };
    char* ptr    = str.rawptr();
    auto ec      = std::to_chars(ptr, ptr + len, value, char_format[fmtc - 'e'], precision);
    size_t wrlen = static_cast<size_t>(ec.ptr - str.rawptr());
    str.set_size(wrlen);
    return upper ? str.upper() : str;
}
String FloatToStrImpl(float value, const char* fmt, int precision) {
    return DoubleToStrImpl(static_cast<double>(value), fmt, precision);
}
}    // namespace ARLib

#pragma once
#include "StringView.hpp"

namespace ARLib {
namespace FormatStringDetail {
    template <typename... Args>
    void compiletime_assertion_fail(Args...);
    template <size_t ArgsSize>
    consteval bool check_format_string(StringView str) {
        size_t count = 0;
        size_t N     = str.size();
        enum class FormatState { EscapeNextOpen, EscapeNextClosed, Continue } state{ FormatState::Continue };
        for (size_t idx = 0; idx < N; idx++) {
            const char c = str[idx];
            if (c == '{') {
                switch (state) {
                    case FormatState::Continue:
                    case FormatState::EscapeNextClosed:
                        state = FormatState::EscapeNextOpen;
                        break;
                    case FormatState::EscapeNextOpen:
                        state = FormatState::Continue;
                        break;
                }
            } else if (c == '}') {
                switch (state) {
                    case FormatState::EscapeNextOpen:
                        count++;
                        [[fallthrough]];
                    case FormatState::EscapeNextClosed:
                        state = FormatState::Continue;
                        break;
                    case FormatState::Continue:
                        state = FormatState::EscapeNextClosed;
                        break;
                }
            } else {
                if (state != FormatState::EscapeNextOpen) { state = FormatState::Continue; }
            }
        }
        return count == ArgsSize;
    }
}    // namespace FormatStringDetail
template <size_t ArgsSize>
struct FormatString {
    StringView fmt;
    template <size_t N>
    consteval FormatString(const char (&fmt_)[N]) : fmt{ fmt_, N - 1 } {
        bool result = FormatStringDetail::check_format_string<ArgsSize>(fmt);
        if (!result)
            FormatStringDetail::compiletime_assertion_fail(
            "Format arguments are not the same number as formats to fill"
            );
    }
    consteval FormatString(StringView fmt_) : fmt{ fmt_ } {
        bool result = FormatStringDetail::check_format_string<ArgsSize>(fmt);
        if (!result)
            FormatStringDetail::compiletime_assertion_fail(
            "Format arguments are not the same number as formats to fill"
            );
    }
    consteval size_t size() const { return fmt.size(); }
    consteval size_t num_args() const { return ArgsSize; }
};
}    // namespace ARLib
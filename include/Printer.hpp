#pragma once
#include "PrintInfo.hpp"
#include "StringView.hpp"
#include "Vector.hpp"
#include "SSOVector.hpp"
#include "cstdio_compat.hpp"
#include "FormatString.hpp"
namespace ARLib {
// anything that wants to be printed from this function has to specialize PrintInfo
class Printer {
    size_t current_index = 0;
    Vector<size_t> indexes{};
    String format_string{};
    String builder{};
    template <Printable Arg, typename... Args>
    void print_impl(const SSOVector<String>& format_specs, const Arg& arg, const Args&... args) {
        if constexpr (PrintableFormatted<Arg>) {
            builder.append(PrintInfo<Arg>{ arg }.repr(format_specs[current_index]));
        } else {
            HARD_ASSERT(
            format_specs[current_index].is_empty(), "Format specifiers are only allowed for PrintableFormatted types"
            );
            builder.append(PrintInfo<Arg>{ arg }.repr());
        }
        if constexpr (sizeof...(args) == 0) {
            builder.append(format_string.substringview(indexes.last() + 2));
        } else {
            builder.append(
            format_string.substringview(indexes.index(current_index) + 2, indexes.index(current_index + 1))
            );
            current_index++;
            print_impl(format_specs, args...);
        }
    }
    template <typename... Args>
    explicit Printer(StringView format, const Args&... args) : format_string{ format } {
        enum class FormatState { EscapeNextOpen, EscapeNextClosed, Continue } state{ FormatState::Continue };
        String escaped_format_string{};
        escaped_format_string.reserve(format.size());
        SSOVector<String, sizeof...(Args)> format_specs{};
        String current_format_spec{};
        for (size_t idx = 0; idx < format_string.size(); idx++) {
            char c = format_string[idx];
            if (c == '{') {
                switch (state) {
                    case FormatState::Continue:
                    case FormatState::EscapeNextClosed:
                        state = FormatState::EscapeNextOpen;
                        escaped_format_string.append(c);
                        break;
                    case FormatState::EscapeNextOpen:
                        state = FormatState::Continue;
                        break;
                }
            } else if (c == '}') {
                switch (state) {
                    case FormatState::EscapeNextOpen:
                        indexes.append(escaped_format_string.size() - 1);
                        escaped_format_string.append(c);
                        current_format_spec.itrim();
                        format_specs.append(move(current_format_spec));
                        [[fallthrough]];
                    case FormatState::EscapeNextClosed:
                        state = FormatState::Continue;
                        break;
                    case FormatState::Continue:
                        state = FormatState::EscapeNextClosed;
                        escaped_format_string.append(c);
                        break;
                }
            } else {
                if (state != FormatState::EscapeNextOpen) {
                    escaped_format_string.append(c);
                    state = FormatState::Continue;
                } else {
                    current_format_spec.append(c);
                }
            }
        }
        format_string = move(escaped_format_string);
        constexpr auto num_args = sizeof...(args);
        builder.reserve(format.size());
        if constexpr (num_args == 0) {
            builder = move(format_string);
        } else {
            builder.append(format_string.substringview(0, indexes.index(current_index)));
            print_impl(format_specs, args...);
        }
    }
    void print_puts() { puts(builder.data()); }

    public:
#ifdef __INTELLISENSE__
    template <typename... Args>
    static void print(StringView format, const Args&... args) {
        if constexpr (sizeof...(args) == 0) {
            puts(format.data());
        } else {
            Printer printer{ format, args... };
            printer.print_puts();
        }
    }
    template <typename... Args>
    [[nodiscard]] static String format(StringView format, const Args&... args) {
        Printer printer{ format, args... };
        return move(printer.builder);
    }
#else
    template <typename... Args>
    static void print(FormatString<sizeof...(Args)>&& format, const Args&... args) {
        if constexpr (sizeof...(args) == 0) {
            puts(format.fmt.data());
        } else {
            Printer printer{ format.fmt, args... };
            printer.print_puts();
        }
    }
    template <typename... Args>
    [[nodiscard]] static String format(FormatString<sizeof...(Args)>&& format, const Args&... args) {
        Printer printer{ format.fmt, args... };
        return move(printer.builder);
    }
#endif
};
}    // namespace ARLib

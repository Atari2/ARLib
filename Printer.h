#pragma once
#include "PrintInfo.h"
#include "StringView.h"
#include "Vector.h"
#include "cstdio_compat.h"

namespace ARLib {
    // anything that wants to be printed from this function has to specialize PrintInfo

    class Printer {
        size_t current_index = 0;
        Vector<size_t> indexes{};
        String format_string{};
        String builder{};

        template <Printable Arg, typename... Args>
        void print_impl(const Arg& arg, const Args&... args) {
            builder.append(PrintInfo<Arg>{arg}.repr());
            if constexpr (sizeof...(args) == 0) {
                builder.append(format_string.substringview(indexes.last() + 2));
            } else {
                builder.append(
                format_string.substringview(indexes.index(current_index) + 2, indexes.index(current_index + 1)));
                current_index++;
                print_impl(args...);
            }
        }

        template <size_t N, typename... Args>
        explicit Printer(const char (&format)[N], const Args&... args) : format_string{format} {
            enum class FormatState { EscapeNextOpen, EscapeNextClosed, Continue } state{FormatState::Continue};
            String escaped_format_string{};
            escaped_format_string.reserve(N);
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
                    escaped_format_string.append(c);
                    state = FormatState::Continue;
                }
            }
            format_string = move(escaped_format_string);
            constexpr auto num_args = sizeof...(args);
            HARD_ASSERT_FMT(
            (indexes.size() == num_args),
            "Format arguments are not the same number as formats to fill, arguments are %d, to fill there are %d",
            num_args, indexes.size())
            builder.reserve(N);
            if constexpr (num_args == 0) {
                builder = move(format_string);
            } else {
                builder.append(format_string.substringview(0, indexes.index(current_index)));
                print_impl(args...);
            }
        }

        void print_puts() { puts(builder.data()); }

        public:
        template <size_t N, typename... Args>
        static void print(const char (&format)[N], const Args&... args) {
            if constexpr (sizeof...(args) == 0) {
                puts(format);
            } else {
                Printer printer{format, args...};
                printer.print_puts();
            }
        }

        template <size_t N, typename... Args>
        [[nodiscard]] static String format(const char (&format)[N], const Args&... args) {
            Printer printer{format, args...};
            return move(printer.builder);
        }
    };
} // namespace ARLib
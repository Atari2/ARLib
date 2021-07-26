#pragma once
#include "PrintInfo.h"
#include "cstdio_compat.h"
namespace ARLib {
    // anything that wants to be printed from this function has to specialize PrintInfo

    class Printer {
        const size_t fmt_len;
        size_t current_index = 0;
        Vector<size_t> indexes{};
        String format_string{};
        String builder{};

        template <Printable Arg, typename... Args>
        void print_impl(const Arg& arg, const Args&... args) {
            builder.concat(PrintInfo<Arg>{arg}.repr());
            if constexpr (sizeof...(args) == 0) {
                builder.concat(format_string.substringview(indexes.last() + 2));
            } else {
                builder.concat(format_string.substringview(indexes[current_index] + 2, indexes[current_index + 1]));
                current_index++;
                print_impl(args...);
            }
        }
        void print_puts() { puts(builder.data()); }

        template <size_t N, typename... Args>
        Printer(const char (&format)[N], const Args&... args) : fmt_len(N), format_string{format} {
            static_assert(sizeof...(args) > 0);
            size_t index = format_string.index_of("{}");
            while (index != String::npos) {
                indexes.append(index);
                index = format_string.index_of("{}", index + 2);
            }
            constexpr auto num_args = sizeof...(args);
            HARD_ASSERT_FMT(
            (indexes.size() == num_args),
            "Format arguments are not the same number as formats to fill, arguments are %d, to fill there are %d",
            num_args, indexes.size());
            builder.reserve(N);
            builder.concat(format_string.substringview(0, indexes[current_index]));
            print_impl(args...);
        }

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
            if constexpr (sizeof...(args) == 0) {
                return String{format};
            } else {
                Printer printer{format, args...};
                return printer.builder;
            }
        }
    };
} // namespace ARLib
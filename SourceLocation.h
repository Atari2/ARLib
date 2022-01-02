#pragma once
#include "Compat.h"
#include "Types.h"
#include "cstdio_compat.h"

namespace ARLib {
    struct SourceLocation {
#ifdef COMPILER_MSVC
#define GCC_COMPAT_COLUMN __builtin_COLUMN()
#else
#if __has_builtin(__builtin_COLUMN)
#define GCC_COMPAT_COLUMN __builtin_COLUMN()
#else
#define GCC_COMPAT_COLUMN 0u
#endif
#endif
        [[nodiscard]] static constexpr SourceLocation
        current(const uint32_t line = __builtin_LINE(), const uint32_t column = GCC_COMPAT_COLUMN,
                const char* const file = __builtin_FILE(), const char* const function = __builtin_FUNCTION()) noexcept {
            SourceLocation res;
            res.line_ = line;
            res.column_ = column;
            res.file_ = file;
            res.function_ = function;
            return res;
        }

        constexpr SourceLocation() noexcept = default;

        [[nodiscard]] constexpr uint32_t line() const noexcept { return line_; }
        [[nodiscard]] constexpr uint32_t column() const noexcept { return column_; }
        [[nodiscard]] constexpr const char* file_name() const noexcept { return file_; }
        [[nodiscard]] constexpr const char* function_name() const noexcept { return function_; }

        private:
        uint32_t line_{};
        uint32_t column_{};
        const char* file_ = "";
        const char* function_ = "";
    };

} // namespace ARLib

#define PRINT_SOURCE_LOCATION                                                                                          \
    ARLib::SourceLocation loc_ = ARLib::SourceLocation::current();                                                     \
    ARLib::printf("Function `%s` in file %s, at line %u and column %u\n", loc_.function_name(), loc_.file_name(),      \
                  loc_.line(), loc_.column());

#pragma once
#include "Compat.hpp"
#include "Types.hpp"
#include "cstdio_compat.hpp"
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
    [[nodiscard]] constexpr static SourceLocation current(
    const uint32_t line = __builtin_LINE(), const uint32_t column = GCC_COMPAT_COLUMN,
    const char* const file = __builtin_FILE(), const char* const function = __builtin_FUNCTION()
    ) noexcept {
        SourceLocation res;
        res.line_     = line;
        res.column_   = column;
        res.file_     = file;
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
    const char* file_     = "";
    const char* function_ = "";
};
void __print_debug_source_location(const SourceLocation&, const char* message);
}    // namespace ARLib
inline void
PRINT_SOURCE_LOCATION(const char* message = nullptr, ARLib::SourceLocation loc_ = ARLib::SourceLocation::current()) {
    ARLib::__print_debug_source_location(loc_, message);
}


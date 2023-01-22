#pragma once
#include "Array.h"
#include "PrintInfo.h"
#include "StringView.h"
#include "cstring_compat.h"
namespace ARLib {

// gcc 11+ and msvc 19.3+ support everything in here
// clang 13 however (latest clang) doesn't, clang 14 (not available yet) does
#ifdef COMPILER_CLANG
    #if __clang_major__ >= 14
        #define STRINGLITERAL_AVAILABLE
    #endif
#else
    #define STRINGLITERAL_AVAILABLE
#endif

#ifdef STRINGLITERAL_AVAILABLE
// this is a weird class
// it attempts at adding a convenience API around a compile time string
// e.g. "hello world"
// all of the operations in this class are done at compile time
// however, due to various constraints around constexpr/consteval
// there are 2 things that you need to do to make this class work
// most return values from this class' member function should be marked constexpr
// this means that you can use return values in static_asserts without issues
// additionally, to be able to use some of this class' API
// you'll need to declare the variable as static (and constexpr) to guarantee storage pointer stability.
template <size_t N>
struct StringLiteral {
    const char _m_str[N];
    constexpr static inline size_t npos   = static_cast<size_t>(-1);
    constexpr static inline size_t m_size = N - 1;
    consteval const char* ptr() const { return _m_str; }
    consteval size_t size() const { return m_size; }
    consteval char operator[](const size_t index) const { return _m_str[index]; }
    template <size_t M>
    consteval size_t count(const char (&c)[M]) const {
        size_t count                = 0;
        constexpr size_t needle_len = M - 1;
        size_t idx                  = 0;
        while (idx < N) {
            bool is_equal = strncmp(_m_str + idx, c, needle_len) == 0;
            count += is_equal;
            idx += is_equal ? needle_len : 1;
        }
        return count;
    }
    /// <summary>
    /// <para>
    /// This function expects the number of occurrences of the separator
    /// as a template parameter
    /// </para>
    /// For example, to split <code>str</code> on spaces: str.split&lt;str.count(" ")&gt;(" ");
    /// </summary>
    /// <param name="c">Separator to split for</param>
    /// <returns>Array of slices of the original string</returns>
    template <size_t CNT, size_t M>
    consteval auto split(const char (&c)[M]) const {
        Array<StringView, CNT + 1> splits{};
        constexpr size_t needle_len = M - 1;
        size_t prev_idx             = 0;
        size_t idx                  = 0;
        size_t c_idx                = 0;
        while (idx < N) {
            bool is_equal = strncmp(_m_str + idx, c, needle_len) == 0;
            if (is_equal) {
                splits[c_idx] = StringView{ _m_str + prev_idx, _m_str + idx };
                prev_idx      = idx + needle_len;
                c_idx++;
            }
            idx += is_equal ? needle_len : 1;
        }
        splits[CNT] = StringView{ _m_str + prev_idx, N - prev_idx - 1 };
        return splits;
    }
    template <size_t M>
    consteval size_t index_of(const char (&c)[M]) const {
        constexpr size_t needle_len = M - 1;
        if (needle_len > m_size) return npos;
        if (needle_len == m_size) return strncmp(_m_str, c, needle_len) == 0 ? 0 : npos;
        size_t idx = 0;
        while (idx < N - needle_len + 1) {
            bool is_equal = strncmp(_m_str + idx, c, needle_len) == 0;
            if (is_equal) return idx;
            idx++;
        }
        return npos;
    }
};
template <size_t N>
StringLiteral(const char (&str)[N]) -> StringLiteral<N>;
template <size_t N>
struct PrintInfo<StringLiteral<N>> {
    const StringLiteral<N>& m_string;
    PrintInfo(const StringLiteral<N>& string) : m_string(string) {}
    String repr() const { return String{ m_string.ptr() }; }
};
#endif
}    // namespace ARLib

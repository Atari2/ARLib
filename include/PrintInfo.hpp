#pragma once
#include "BaseTraits.hpp"
#include "Concepts.hpp"
#include "SourceLocation.hpp"
#include "TypeInfo.hpp"
#include "String.hpp"
namespace ARLib {
template <typename T>
struct PrintInfo;

template <typename T>
PrintInfo(T) -> PrintInfo<T>;

template <typename T>
concept Printable = requires() {
                        { declval<PrintInfo<T>>().repr() } -> SameAs<String>;
                    };
template <typename T>
String print_conditional(const T& m_value) {
    if constexpr (Printable<T>) {
        return PrintInfo<T>{ m_value }.repr();
    } else {
        DemangledInfo info{ MANGLED_TYPENAME_TO_STRING(T), false };
        return String{ info.name() };
    }
};
#define BASIC_NOT_CONST_PRINT_IMPL(x, impl)                                                                            \
    template <>                                                                                                        \
    struct PrintInfo<x> {                                                                                              \
        using type_ = AddConstT<AddLValueRefIfNotPtrT<x>>;                                                             \
        type_ m_val;                                                                                                   \
        explicit PrintInfo(type_ val) : m_val(val) {}                                                                  \
        String repr() const { return impl(m_val); }                                                                    \
    };

#define BASIC_CONST_PRINT_IMPL(x, impl)                                                                                \
    template <>                                                                                                        \
    struct PrintInfo<const x> {                                                                                        \
        using type_ = AddConstT<AddLValueRefIfNotPtrT<const x>>;                                                       \
        type_ m_val;                                                                                                   \
        explicit PrintInfo(type_ val) : m_val(val) {}                                                                  \
        String repr() const { return impl(m_val); }                                                                    \
    };

#define BASIC_PRINT_IMPL(x, impl)                                                                                      \
    BASIC_NOT_CONST_PRINT_IMPL(x, impl)                                                                                \
    BASIC_CONST_PRINT_IMPL(x, impl)

BASIC_PRINT_IMPL(String, )
BASIC_PRINT_IMPL(char*, String)
template <typename T>
struct PrintInfo<T*> {
    const T* m_ptr;
    explicit PrintInfo(const T* ptr) : m_ptr(ptr) {}
    String repr() {
        if constexpr (Printable<T>) {
            if (m_ptr)
                return String::formatted("0x%p -> ", m_ptr) + PrintInfo<T>{ *m_ptr }.repr();
            else
                return "nullptr"_s;
        } else {
            DemangledInfo info{ MANGLED_TYPENAME_TO_STRING(T), false };
            if (m_ptr)
                return String::formatted("0x%p (pointer to %s)", m_ptr, info.name());
            else
                return "nullptr"_s + " (pointer to "_s + info.name() + ')';
        }
    }
};
template <Printable T, size_t N>
struct PrintInfo<T[N]> {
    const T (&m_vec)[N];
    explicit PrintInfo(const T (&vec)[N]) : m_vec(vec) {}
    String repr() {
        String str{ "[" };
        for (size_t i = 0; i < N; i++) {
            str.append(PrintInfo<T>{ m_vec[i] }.repr());
            if (i != N - 1) {
                str.append(',');
                str.append(' ');
            }
        }
        str.append(']');
        return str;
    }
};
template <size_t N>
struct PrintInfo<char[N]> {
    const char (&m_str)[N];
    explicit PrintInfo(const char (&str)[N]) : m_str(str) {}
    String repr() const { return String{ m_str }; }
};
template <Printable T, size_t N, size_t M>
requires(!IsArrayV<T>)
struct PrintInfo<T[N][M]> {
    const T(&m_matrix)[N][M];
    explicit PrintInfo(const T(&matrix)[N][M]) : m_matrix(matrix) {}
    String repr() {
        String str{ "[\n" };
        for (size_t i = 0; i < N; i++) {
            str.append('\t');
            str.append(PrintInfo<T[M]>{ m_matrix[i] }.repr());
            if (i != N - 1) { str.append(','); }
            str.append('\n');
        }
        str.append(']');
        return str;
    }
};
template <>
struct PrintInfo<SourceLocation> {
    const SourceLocation& m_loc;
    explicit PrintInfo(const SourceLocation& loc) : m_loc(loc) {}
    String repr() const;
};
template <>
struct PrintInfo<DemangledInfo> {
    const DemangledInfo& m_info;
    explicit PrintInfo(const DemangledInfo& info) : m_info(info) {}
    String repr() const;
};
template <>
struct PrintInfo<nullptr_t> {
    explicit PrintInfo(nullptr_t) {}
    String repr() const { return "nullptr"_s; }
};
}    // namespace ARLib

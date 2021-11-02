#pragma once
#include "BaseTraits.h"
#include "Concepts.h"
#include "SourceLocation.h"
#include "String.h"

namespace ARLib {
    template <typename T>
    struct PrintInfo {};

    template <typename T>
    concept Printable = requires(const T& a) {
        { PrintInfo<T>{a}.repr() } -> SameAs<String>;
    };

    template <typename T>
    String print_conditional(const T& m_value) {
        if constexpr (Printable<T>) {
            return PrintInfo<T>{m_value}.repr();
        } else {
            DemangledInfo info{MANGLED_TYPENAME_TO_STRING(T)};
            return String{info.name()};
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
                return String::formatted("0x%p -> ", m_ptr) + PrintInfo<T>{*m_ptr}.repr();
            } else {
                DemangledInfo info{MANGLED_TYPENAME_TO_STRING(T), false};
                return String::formatted("0x%p (pointer to %s)", m_ptr, info.name());
            }
        }
    };

    template <Printable T, size_t N>
    struct PrintInfo<T[N]> {
        const T (&m_vec)[N];
        explicit PrintInfo(const T (&vec)[N]) : m_vec(vec) {}
        String repr() {
            String str{"["};
            for (size_t i = 0; i < N; i++) {
                str.append(PrintInfo<T>{m_vec[i]}.repr());
                if (i != N - 1) {
                    str.append(',');
                    str.append(' ');
                }
            }
            str.append(']');
            return str;
        }
    };

    template <Printable T, size_t N, size_t M>
    requires(!IsArrayV<T>) struct PrintInfo<T[N][M]> {
        const T (&m_matrix)[N][M];
        explicit PrintInfo(const T (&matrix)[N][M]) : m_matrix(matrix) {}
        String repr() {
            String str{"[\n"};
            for (size_t i = 0; i < N; i++) {
                str.append('\t');
                str.append(PrintInfo<T[M]>{m_matrix[i]}.repr());
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
        String repr();
    };

    template <>
    struct PrintInfo<nullptr_t> {
        explicit PrintInfo(nullptr_t) {}
        String repr() const { return "nullptr"_s; }
    };

} // namespace ARLib
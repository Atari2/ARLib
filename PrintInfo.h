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

#define BASIC_NOT_CONST_PRINT_IMPL(x, impl)                                                                            \
    template <>                                                                                                        \
    struct PrintInfo<x> {                                                                                              \
        using type_ = AddConstT<AddLValueRefIfNotPtrT<x>>;                                                             \
        type_ m_val;                                                                                                   \
        PrintInfo(type_ val) : m_val(val) {}                                                                           \
        String repr() const { return impl(m_val); }                                                                    \
    };

#define BASIC_CONST_PRINT_IMPL(x, impl)                                                                                \
    template <>                                                                                                        \
    struct PrintInfo<const x> {                                                                                        \
        using type_ = AddConstT<AddLValueRefIfNotPtrT<const x>>;                                                       \
        type_ m_val;                                                                                                   \
        PrintInfo(type_ val) : m_val(val) {}                                                                           \
        String repr() const { return impl(m_val); }                                                                    \
    };

#define BASIC_PRINT_IMPL(x, impl)                                                                                      \
    BASIC_NOT_CONST_PRINT_IMPL(x, impl)                                                                                \
    BASIC_CONST_PRINT_IMPL(x, impl)

    BASIC_PRINT_IMPL(String, )
    BASIC_PRINT_IMPL(char*, )

    template <typename T>
    struct PrintInfo<T*> {
        const T* m_ptr;
        PrintInfo(const T* ptr) : m_ptr(ptr) {}
        String repr() {
            if constexpr (Printable<T>) {
                return String::formatted("0x%p -> ", m_ptr) + PrintInfo<T>{*m_ptr}.repr();
            } else {
                return String::formatted("0x%p (pointer to %s)", m_ptr, typeid(T).name());
            }
        }
    };

    template <Printable T, size_t N>
    struct PrintInfo<T[N]> {
        const T (&m_vec)[N];
        PrintInfo(const T (&vec)[N]) : m_vec(vec) {}
        String repr() {
            String str{"["};
            for (size_t i = 0; i < N; i++) {
                str.concat(PrintInfo<T>{m_vec[i]}.repr());
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
        PrintInfo(const T (&matrix)[N][M]) : m_matrix(matrix) {}
        String repr() {
            String str{"[\n"};
            for (size_t i = 0; i < N; i++) {
                str.append('\t');
                str.concat(PrintInfo<T[M]>{m_matrix[i]}.repr());
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
        PrintInfo(const SourceLocation& loc) : m_loc(loc) {}
        String repr();
    };

} // namespace ARLib
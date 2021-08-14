#pragma once
#include "CharConv.h"
#include "Concepts.h"
#include "Functional.h"
#include "Map.h"

namespace ARLib {
    template <typename T>
    struct PrintInfo {};

    template <typename T>
    concept Printable = requires(const T& a) {
        { PrintInfo<T>{a}.repr() } -> SameAs<String>;
    };

#define BASIC_PRINT_IMPL(x, impl)                                                                                      \
    template <>                                                                                                        \
    struct PrintInfo<x> {                                                                                              \
        const x& m_val;                                                                                                \
        PrintInfo(const x& val) : m_val(val) {}                                                                        \
        String repr() const { return impl(m_val); }                                                                    \
    };

    BASIC_PRINT_IMPL(int, IntToStr)
    BASIC_PRINT_IMPL(double, DoubleToStr)
    BASIC_PRINT_IMPL(String, [](const String& a) { return a; })
    BASIC_PRINT_IMPL(size_t, IntToStr)

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
    struct PrintInfo<T[N][M]> {
        const T (&m_matrix)[N][M];
        PrintInfo(const T (&matrix)[N][M]) : m_matrix(matrix) {}
        String repr() {
            String str{"[\n"};
            for (size_t i = 0; i < N; i++) {
                str.append('\t');
                str.concat(PrintInfo<T[M]>{m_matrix[i]}.repr());
                if (i != N - 1) {
                    str.append(',');
                }
                str.append('\n');
            }
            str.append(']');
            return str;
        }
    };

    template <Printable T>
    struct PrintInfo<Vector<T>> {
        const Vector<T>& m_vec;
        PrintInfo(const Vector<T>& vec) : m_vec(vec) {}
        String repr() {
            if (m_vec.size() == 0) { return "[]"_s; }
            String con{};
            if constexpr (IsSameV<T, String>) {
                for (const auto& s : m_vec) {
                    con.concat("[\"");
                    con.concat(PrintInfo<T>{s}.repr());
                    con.concat("\"], ");
                }
            } else {
                for (const auto& s : m_vec) {
                    con.append('[');
                    con.concat(PrintInfo<T>{s}.repr());
                    con.concat("], ");
                }
            }

            return con.substring(0, con.size() - 2);
        }
    };

    template <Printable A, Printable B>
    struct PrintInfo<Map<A, B>> {
        const Map<A, B>& m_map;
        PrintInfo(const Map<A, B>& map) : m_map(map) {}
        String repr() const {
            if (m_map.size() == 0) { return "{}"_s; }
            String con{};
            con.concat("{ ");
            for (const auto& [key, val] : m_map) {
                con.concat(PrintInfo<A>{key}.repr());
                con.concat(": "_s);
                con.concat(PrintInfo<B>{val}.repr());
                con.concat(", ");
            }
            return con.substring(0, con.size() - 2) + " }"_s;
        }
    };

    template <typename Arg, typename... Args>
    struct PrintInfo<detail::PartialArguments<Arg, Args...>> {
        const detail::PartialArguments<Arg, Args...>& m_partial;
        PrintInfo(const detail::PartialArguments<Arg, Args...>& partial) : m_partial(partial) {}
        String repr() {
            if constexpr (sizeof...(Args) == 0) {
                return String{typeid(Arg).name()};
            } else {
                using Inner = detail::PartialArguments<Args...>;
                return String{typeid(Arg).name()} + ", "_s +
                       PrintInfo<Inner>{static_cast<const Inner&>(m_partial)}.repr();
            }
        }
    };

    template <typename Func, typename... Args>
    struct PrintInfo<PartialFunction<Func, Args...>> {
        const PartialFunction<Func, Args...>& m_func;
        PrintInfo(const PartialFunction<Func, Args...>& func) : m_func(func) {}
        String repr() {
            return "PartialFunction "_s + String{typeid(Func).name()} + " with partial arguments: ("_s +
                   PrintInfo<detail::PartialArguments<Args...>>{m_func.partial_args()}.repr() + ")"_s;
        }
    };

} // namespace ARLib
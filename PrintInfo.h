#pragma once
#include "CharConv.h"
#include "Concepts.h"
#include "Map.h"

namespace ARLib {
    template <typename T>
    struct PrintInfo {};

    template <typename T>
    concept Printable = requires(const T& a) {
        { PrintInfo<T>{a}.repr() } -> SameAs<String>;
    };

    template <>
    struct PrintInfo<int> {
        const int m_int;
        PrintInfo(const int num) : m_int(num) {}
        String repr() const { return IntToStr(m_int); }
    };

    template <>
    struct PrintInfo<double> {
        const double m_double;
        PrintInfo(const double num) : m_double(num) {}
        String repr() const { return DoubleToStr(m_double); }
    };

    template <>
    struct PrintInfo<String> {
        const String& m_str;
        PrintInfo(const String& str) : m_str(str) {}
        String repr() const { return m_str; }
    };

    template <Printable T>
    struct PrintInfo<Vector<T>> {
        const Vector<T>& m_vec;
        PrintInfo(const Vector<T>& vec) : m_vec(vec) {}
        String repr() {
            String con{};
            for (const auto& s : m_vec) {
                con.append('[');
                con.concat(PrintInfo<T>{s}.repr());
                con.concat("], ");
            }
            return con.substring(0, con.size() - 2);
        }
    };

    template <Printable A, Printable B>
    struct PrintInfo<Map<A, B>> {
        const Map<A, B>& m_map;
        PrintInfo(const Map<A, B>& map) : m_map(map) {}
        String repr() const {
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

} // namespace ARLib
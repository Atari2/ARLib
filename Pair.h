#pragma once
#include "Types.h"
#include "Utility.h"

namespace ARLib {
    template <typename T, typename U>
    struct Pair {
        T m_first;
        U m_second;

        Pair() = default;
        Pair(const T& first, const U& second) : m_first(first), m_second(second) {}
        Pair(T&& first, U&& second) : m_first(move(first)), m_second(move(second)) {}

        Pair(const Pair&) = default;
        Pair(Pair&&) = default;
        Pair& operator=(Pair&&) = default;
        Pair& operator=(const Pair&) = default;

        T& first() { return m_first; }
        U& second() { return m_second; }

        const T& first() const { return m_first; }
        const U& second() const { return m_second; }

        template <size_t Index>
        auto& get() & {
            static_assert(Index == 0 || Index == 1);
            if constexpr (Index == 0)
                return m_first;
            else if constexpr (Index == 1)
                return m_second;
        }

        template <size_t Index>
        auto const& get() const& {
            static_assert(Index == 0 || Index == 1);
            if constexpr (Index == 0)
                return m_first;
            else if constexpr (Index == 1)
                return m_second;
        }

        template <size_t Index>
        auto&& get() && {
            static_assert(Index == 0 || Index == 1);
            if constexpr (Index == 0)
                return move(m_first);
            else if constexpr (Index == 1)
                return move(m_second);
        }

        ~Pair() = default;
    };

    template <typename T, typename U>
    struct Pair<T&, U&> {
        T& m_first;
        U& m_second;

        Pair(T& first, U& second) : m_first(first), m_second(second) {}

        Pair(const Pair& other) : m_first(other.m_first), m_second(other.m_second){};
        Pair(Pair&& other) : m_first(other.m_first), m_second(other.m_second){};

        T& first() { return m_first; }
        U& second() { return m_second; }

        const T& first() const { return m_first; }
        const U& second() const { return m_second; }

        template <size_t Index>
        auto& get() & {
            static_assert(Index == 0 || Index == 1);
            if constexpr (Index == 0)
                return m_first;
            else if constexpr (Index == 1)
                return m_second;
        }

        template <size_t Index>
        auto const& get() const& {
            static_assert(Index == 0 || Index == 1);
            if constexpr (Index == 0)
                return m_first;
            else if constexpr (Index == 1)
                return m_second;
        }

        template <size_t Index>
        auto&& get() && {
            static_assert(Index == 0 || Index == 1);
            if constexpr (Index == 0)
                return move(m_first);
            else if constexpr (Index == 1)
                return move(m_second);
        }

        ~Pair() = default;
    };

} // namespace ARLib

using ARLib::Pair;
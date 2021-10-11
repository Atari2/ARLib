#pragma once
#include "Iterator.h"
#include "PrintInfo.h"
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
        Pair(Pair&&) noexcept = default;
        Pair& operator=(Pair&&) noexcept = default;
        Pair& operator=(const Pair&) = default;

        bool operator==(const Pair& other) const { return m_first == other.m_first && m_second == other.m_second; }
        bool operator!=(const Pair& other) const { return m_first != other.m_first || m_second != other.m_second; }

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
        Pair(Pair&& other) noexcept : m_first(other.m_first), m_second(other.m_second){};

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

    template <Printable A, Printable B>
    struct PrintInfo<Pair<A, B>> {
        const Pair<A, B>& m_pair;
        explicit PrintInfo(const Pair<A, B>& pair) : m_pair(pair) {}
        String repr() const {
            return "{ "_s + PrintInfo<A>{m_pair.first()}.repr() + ", "_s + PrintInfo<B>{m_pair.second()}.repr() +
                   " }"_s;
        }
    };

    template <typename F, typename S>
    class PairIterator {
        using IterUnit = Pair<F, S>;
        IterUnit m_current_pair;
        using FT = decltype(*m_current_pair.template get<0>());
        using ST = decltype(*m_current_pair.template get<1>());

        public:
        PairIterator(F first, S second) : m_current_pair(first, second) {}
        explicit PairIterator(IterUnit curr_pair) : m_current_pair(curr_pair){};
        Pair<FT, ST> operator*() { return {*m_current_pair.template get<0>(), *m_current_pair.template get<1>()}; }
        PairIterator& operator++() {
            m_current_pair.template get<0>()++;
            m_current_pair.template get<1>()++;
            return *this;
        }

        PairIterator operator++(int) {
            auto copy = *this;
            this->operator++();
            return copy;
        }
        bool operator==(const PairIterator& other) const { return m_current_pair == other.m_current_pair; }
        bool operator!=(const PairIterator& other) const { return m_current_pair != other.m_current_pair; }
        bool operator<(const PairIterator& other) {
            return m_current_pair.template get<0>() < other.m_current_pair.template get<0>() &&
                   m_current_pair.template get<1>() < other.m_current_pair.template get<1>();
        }
        bool operator>(const PairIterator& other) {
            return m_current_pair.template get<0>() > other.m_current_pair.template get<0>() &&
                   m_current_pair.template get<1>() > other.m_current_pair.template get<1>();
        }
    };

    template <typename T>
    class Enumerator : public IteratorOperators<Enumerator<T>>, IteratorType<T> {
        Iterator<T> m_iter;
        size_t m_index;

        using Unit = Pair<size_t, T&>;

        public:
        explicit Enumerator(T* begin) : m_iter(begin), m_index(0) {}
        Enumerator(T* begin, size_t index) : m_iter(begin), m_index(index) {}
        Enumerator(Iterator<T> iter, size_t index) : m_iter(iter), m_index(index) {}
        Unit operator*() { return {m_index, *m_iter}; }

        Enumerator& operator++() {
            m_index++;
            m_iter++;
            return *this;
        }

        Enumerator operator++(int) {
            auto copy = *this;
            this->operator++();
            return copy;
        }
        bool operator==(const Enumerator& other) const override { return m_index == other.m_index; }
        bool operator!=(const Enumerator& other) const override { return m_index != other.m_index; }
        bool operator<(const Enumerator& other) override { return m_index < other.m_index; }
        bool operator>(const Enumerator& other) override { return m_index > other.m_index; }
        size_t operator-(const Enumerator& other) { return m_iter - other.m_iter; }
    };

    template <typename T>
    class ConstEnumerator : public IteratorOperators<ConstEnumerator<T>>, IteratorType<T> {
        ConstIterator<T> m_iter;
        size_t m_index;

        using Unit = Pair<size_t, const T&>;

        public:
        explicit ConstEnumerator(const T* begin) : m_iter(begin), m_index(0) {}
        ConstEnumerator(const T* begin, size_t index) : m_iter(begin), m_index(index) {}
        ConstEnumerator(ConstIterator<T> iter, size_t index) : m_iter(iter), m_index(index) {}
        Unit operator*() { return {m_index, *m_iter}; }

        ConstEnumerator& operator++() {
            m_index++;
            m_iter++;
            return *this;
        }

        ConstEnumerator operator++(int) {
            auto copy = *this;
            this->operator++();
            return copy;
        }
        bool operator==(const ConstEnumerator& other) const override { return m_index == other.m_index; }
        bool operator!=(const ConstEnumerator& other) const override { return m_index != other.m_index; }
        bool operator<(const ConstEnumerator& other) override { return m_index < other.m_index; }
        bool operator>(const ConstEnumerator& other) override { return m_index > other.m_index; }
        size_t operator-(const ConstEnumerator& other) { return m_iter - other.m_iter; }
    };

} // namespace ARLib

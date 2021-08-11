#pragma once
#include "Concepts.h"
#include "Iterator.h"
#include "cmath_compat.h"

namespace ARLib {

    template <typename T>
    constexpr inline auto sum_default = [](const T& elem) {
        return elem;
    };

    constexpr inline size_t npos_ = static_cast<size_t>(-1);

    template <typename C, size_t N>
    consteval size_t sizeof_array(C (&)[N]) {
        return N;
    }

    template <IteratorConcept Iter>
    requires EqualityComparable<typename Iter::Type> size_t find(Iter begin, Iter end,
                                                                 const typename Iter::Type& elem) {
        if (begin == end) return npos_;
        size_t index = 0;
        for (; begin != end; begin++, index++)
            if (*begin == elem) return index;
        return npos_;
    }

    template <typename C, typename T>
    requires Iterable<C>
    auto find(const C& cont, const T& elem) { return find(cont.begin(), cont.end(), elem); }

    template <IteratorConcept Iter>
    requires MoreComparable<typename Iter::Type> Iter max(Iter begin, Iter end) {
        if (begin == end) return begin;
        Iter value{begin};
        for (; begin != end; begin++)
            if (*begin > *value) value = begin;
        return value;
    }

    template <IteratorConcept Iter>
    requires LessComparable<typename Iter::Type> Iter min(Iter begin, Iter end) {
        if (begin == end) return begin;
        Iter value{begin};
        for (; begin != end; begin++)
            if (*begin < *value) value = begin;
        return value;
    }

    template <IteratorConcept Iter, typename Functor = decltype(sum_default<typename Iter::Type>)>
    auto sum(Iter begin, Iter end, Functor func = sum_default<typename Iter::Type>) {
        if (begin == end) return InvokeResultT<Functor, decltype(*begin)>{};
        auto total = func(*begin);
        begin++;
        for (; begin != end; begin++)
            total += func(*begin);
        return total;
    }

    template <typename C>
    requires Iterable<C>
    auto max(const C& cont) { return max(cont.begin(), cont.end()); }

    template <typename C>
    requires Iterable<C>
    auto min(const C& cont) { return min(cont.begin(), cont.end()); }

    template <typename C, typename Functor>
    requires Iterable<C>
    auto sum(const C& cont, Functor func) { return sum(cont.begin(), cont.end(), func); }

    template <typename C, typename Functor>
    requires Iterable<C> && Pushable<C, decltype(*C{}.begin())> &&
    ConvertibleTo<decltype(*C{}.begin()), ResultOfT<Functor(decltype(*C{}.begin()))>>
    auto transform(const C& cont, Functor func) {
        C copy{};
        if constexpr (Reservable<C> && CanKnowSize<C>) { copy.reserve(cont.size()); }
        for (auto& i : cont) {
            copy.push_back(move(func(i)));
        }
        return copy;
    }

    // follows very naive quicksort implementation
    template <IteratorConcept Iter>
    requires MoreComparable<typename Iter::Type> && LessComparable<typename Iter::Type> Iter partition(Iter lo,
                                                                                                       Iter hi) {
        auto pivot = lo + static_cast<int>(((hi - lo) / 2));
        auto i = lo - 1;
        auto j = hi + 1;
        for (;;) {
            do {
                i++;
            } while (*i < *pivot);
            do {
                j--;
            } while (*j > *pivot);
            if (i > j || i == j) { return j; }
            auto item = *i;
            *i = *j;
            *j = item;
        }
    }

    template <IteratorConcept Iter>
    requires MoreComparable<typename Iter::Type> && LessComparable<typename Iter::Type>
    void quicksort_internal(Iter lo, Iter hi) {
        if (lo < hi) {
            auto p = partition(lo, hi);
            quicksort_internal(lo, p);
            quicksort_internal(p + 1, hi);
        }
    }

    // in-place sorting
    template <typename C>
    requires Iterable<C>
    void sort(const C& cont) { quicksort_internal(cont.begin(), cont.end() - 1); }

    template <Iterable C>
    auto begin(const C& cont) {
        return cont.begin();
    }

    template <Iterable C>
    auto end(const C& cont) {
        return cont.end();
    }

    template <typename C, size_t N>
    C* begin(C (&cont)[N]) {
        return PointerTraits<C*>::pointer_to(*cont);
    }

    template <typename C, size_t N>
    C* end(C (&cont)[N]) {
        return PointerTraits<C*>::pointer_to(*cont) + N;
    }

    constexpr auto bit_round(Integral auto v) {
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v++;
        return v;
    }

    auto bit_round_growth(Sized auto requested_size) {
        if (requested_size == 0) return static_cast<decltype(requested_size)>(2);
        size_t ret = bit_round(requested_size);
        return ret;
    }

    auto basic_growth(Sized auto requested_size) {
        if (requested_size < 4096) {
            return bit_round_growth(requested_size);
        } else
            return requested_size + 2048;
    }
    size_t prime_generator(size_t n);

    template <typename T, Container<T> Cont, size_t N>
    constexpr void fill(Cont& container) requires DefaultConstructible<T> {
        if (container.size() >= N) return;
        if constexpr (Resizeable<Cont>) {
            container.resize(N);
        } else {
            size_t prev_size = container.size();
            container.set_size(N);
            for (size_t i = prev_size; i < N; i++)
                container[i] = {};
        }
    }

    template <typename T, Container<T> Cont>
    constexpr void fill(Cont& container, size_t num) requires DefaultConstructible<T> {
        if (container.size() >= num) return;
        if constexpr (Resizeable<Cont>) {
            container.resize(num);
        } else {
            size_t prev_size = container.size();
            container.set_size(num);
            for (size_t i = prev_size; i < num; i++)
                container[i] = {};
        }
    }

    template <typename T, Container<T> Cont, typename... Args>
    constexpr void fill_with(Cont& container, size_t num,
                             Args... args) requires MoveAssignable<T> && Constructible<T, Args...> {
        if (container.size() >= num) return;
        size_t prev_size = container.size();
        container.set_size(num);
        for (size_t i = prev_size; i < num; i++)
            container[i] = move(T{args...});
    }
} // namespace ARLib

using ARLib::fill;
using ARLib::fill_with;
using ARLib::find;
using ARLib::max;
using ARLib::min;
using ARLib::prime_generator;
using ARLib::sort;
using ARLib::sum;

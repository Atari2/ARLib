#pragma once
#include "Concepts.h"
#include "Iterator.h"
#include "IteratorInspection.h"
#include "Utility.h"
#include "cmath_compat.h"
namespace ARLib {

template <typename T>
constexpr inline auto sum_default = [](const T& elem) {
    return elem;
};

constexpr inline size_t npos_ = static_cast<size_t>(-1);
template <IteratorConcept Iter>
requires EqualityComparable<IteratorOutputType<Iter>>
constexpr size_t find(Iter begin, Iter end, const IteratorOutputType<Iter>& elem) {
    if (begin == end) return npos_;
    size_t index = 0;
    for (; begin != end; ++begin, index++)
        if (*begin == elem) return index;
    return npos_;
}
template <typename C, typename T>
requires Iterable<C>
constexpr auto find(const C& cont, const T& elem) {
    return find(cont.begin(), cont.end(), elem);
}
template <typename C, typename Func>
requires Iterable<C> && CallableWithRes<Func, bool, decltype(*declval<C>().begin())>
constexpr auto find_if(const C& cont, Func condition) {
    auto begin     = cont.begin();
    const auto end = cont.end();
    if (begin == end) return npos_;
    size_t index = 0;
    for (; begin != end; ++begin, index++)
        if (condition(*begin)) return index;
    return npos_;
}
template <IteratorConcept Iter>
requires MoreComparable<IteratorOutputType<Iter>>
constexpr Iter max(Iter begin, Iter end) {
    if (begin == end) return begin;
    Iter value{ begin };
    for (; begin != end; ++begin)
        if (*begin > *value) value = begin;
    return value;
}
template <IteratorConcept Iter>
requires LessComparable<IteratorOutputType<Iter>>
constexpr Iter min(Iter begin, Iter end) {
    if (begin == end) return begin;
    Iter value{ begin };
    for (; begin != end; ++begin)
        if (*begin < *value) value = begin;
    return value;
}
template <IteratorConcept Iter, typename Functor = decltype(sum_default<IteratorOutputType<Iter>>)>
constexpr auto sum(Iter begin, Iter end, Functor func = sum_default<IteratorOutputType<Iter>>) {
    if (begin == end) return InvokeResultT<Functor, decltype(*begin)>{};
    auto total = func(*begin);
    begin++;
    for (; begin != end; ++begin) total += func(*begin);
    return total;
}
template <IteratorConcept Iter, bool ROUND = false>
constexpr auto avg(Iter begin, Iter end) {
    size_t sz = 0;
    if constexpr (ROUND) {
        RemoveReferenceT<decltype(*begin)> total{ 0 };
        static_assert(IsIntegralV<decltype(total)>, "Average can only be called on containers of integral types");
        for (; begin != end; ++begin, sz++) { total += *begin; }
        return static_cast<size_t>(total) / sz;
    } else {
        double total{ 0.0 };
        for (; begin != end; ++begin, sz++) { total += static_cast<double>(*begin); }
        return total / static_cast<double>(sz);
    }
}
template <IteratorConcept Iter, typename Functor>
requires CallableWithRes<Functor, bool, decltype(*declval<Iter>())>
constexpr auto all_of(Iter begin, Iter end, Functor func) {
    for (; begin != end; ++begin) {
        if (!func(*begin)) return false;
    }
    return true;
}
template <IteratorConcept Iter, typename Functor>
requires CallableWithRes<Functor, bool, decltype(*declval<Iter>())>
constexpr auto any_of(Iter begin, Iter end, Functor func) {
    for (; begin != end; ++begin) {
        if (func(*begin)) return true;
    }
    return false;
}
template <IteratorConcept Iter, typename Functor>
requires CallableWithRes<Functor, bool, decltype(*declval<Iter>())>
constexpr auto exactly_n(Iter begin, Iter end, Functor func, size_t n) {
    size_t how_many = 0;
    for (; begin != end; ++begin) {
        if (func(*begin)) how_many++;
    }
    return how_many == n;
}
template <typename T>
requires(LessComparable<T> || MoreComparable<T>)
constexpr auto max_bt(const T& left, const T& right) {
    if constexpr (LessComparable<T>) {
        if (left < right)
            return right;
        else
            return left;
    } else {
        if (left > right)
            return left;
        else
            return right;
    }
}
template <typename T>
requires(LessComparable<T> || MoreComparable<T>)
constexpr auto min_bt(const T& left, const T& right) {
    if constexpr (LessComparable<T>) {
        if (left < right)
            return left;
        else
            return right;
    } else {
        if (left > right)
            return right;
        else
            return left;
    }
}
template <typename C>
requires Iterable<C>
constexpr auto max(const C& cont) {
    return max(cont.begin(), cont.end());
}
template <typename C>
requires Iterable<C>
constexpr auto min(const C& cont) {
    return min(cont.begin(), cont.end());
}
template <typename C, typename Functor>
requires Iterable<C>
constexpr auto sum(const C& cont, Functor func) {
    return sum(cont.begin(), cont.end(), func);
}
template <typename C, bool ROUND = false>
requires Iterable<C>
constexpr auto avg(const C& cont) {
    if constexpr (CanKnowSize<C>) {
        if constexpr (ROUND) {
            return static_cast<decltype(cont.size())>(sum(cont, [](auto& i) { return i; })) / cont.size();
        } else {
            return static_cast<double>(sum(cont, [](auto& i) { return i; })) / static_cast<double>(cont.size());
        }
    } else {
        return avg<decltype(cont.begin()), ROUND>(cont.begin(), cont.end());
    }
}
template <Iterable C, typename Functor>
requires CallableWithRes<Functor, bool, decltype(*declval<C>().begin())>
constexpr auto all_of(const C& cont, Functor func) {
    return all_of(cont.begin(), cont.end(), func);
}
template <Iterable C, typename Functor>
requires CallableWithRes<Functor, bool, decltype(*declval<C>().begin())>
constexpr auto any_of(const C& cont, Functor func) {
    return any_of(cont.begin(), cont.end(), func);
}
template <Iterable C, typename Functor>
requires CallableWithRes<Functor, bool, decltype(*declval<C>().begin())>
constexpr auto exactly_n(const C& cont, Functor func, size_t n) {
    return exactly_n(cont.begin(), cont.end(), func, n);
}
template <typename C, typename Functor>
requires Iterable<C> && Pushable<C, decltype(*declval<C>().begin())> &&
         ConvertibleTo<decltype(*C{}.begin()), ResultOfT<Functor(decltype(*C{}.begin()))>>
auto transform(const C& cont, Functor func) {
    C copy{};
    if constexpr (Reservable<C> && CanKnowSize<C>) { copy.reserve(cont.size()); }
    for (auto& i : cont) { copy.push_back(move(func(i))); }
    return copy;
}
// follows very naive quicksort implementation
template <IteratorConcept Iter>
requires MoreComparable<IteratorOutputType<Iter>> && LessComparable<IteratorOutputType<Iter>> &&
         CopyConstructible<RemoveReferenceT<IteratorOutputType<Iter>>>
Iter partition(Iter lo, Iter hi) {
    // FIXME: find a way to avoid this copy of the pivot
    auto pivot = *(lo + ((hi - lo) / 2));
    auto i                         = lo - 1;
    auto j                         = hi + 1;
    for (;;) {
        do { ++i; } while (*i < pivot);
        do { --j; } while (*j > pivot);
        if (i > j || i == j) return j;
        swap(*i, *j);
    }
}
template <IteratorConcept Iter>
requires MoreComparable<IteratorOutputType<Iter>> && LessComparable<IteratorOutputType<Iter>>
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
void sort(C& cont) {
    quicksort_internal(cont.begin(), cont.end() - 1);
}
template <Iterable C>
constexpr auto begin(C& cont) {
    return cont.begin();
}
template <Iterable C>
constexpr auto end(C& cont) {
    return cont.end();
}
template <Iterable C>
constexpr auto begin(const C& cont) {
    return cont.begin();
}
template <Iterable C>
constexpr auto end(const C& cont) {
    return cont.end();
}
template <typename C, size_t N>
constexpr C* begin(C (&cont)[N]) {
    return PointerTraits<C*>::pointer_to(*cont);
}
template <typename C, size_t N>
constexpr C* end(C (&cont)[N]) {
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
constexpr auto bit_round_growth(Sized auto requested_size) {
    if (requested_size == 0) return static_cast<decltype(requested_size)>(2);
    size_t ret = bit_round(requested_size);
    return ret;
}
constexpr auto basic_growth(Sized auto requested_size) {
    if (requested_size < 4096) {
        return bit_round_growth(requested_size);
    } else
        return requested_size + 2048;
}
size_t prime_generator(size_t n);
template <typename T, Container<T> Cont, size_t N>
constexpr void fill(Cont& container)
requires DefaultConstructible<T>
{
    if (container.size() >= N) return;
    if constexpr (Resizeable<Cont>) {
        container.resize(N);
    } else {
        size_t prev_size = container.size();
        container.set_size(N);
        for (size_t i = prev_size; i < N; i++) container[i] = {};
    }
}
template <typename T, Container<T> Cont>
constexpr void fill(Cont& container, size_t num)
requires DefaultConstructible<T>
{
    if (container.size() >= num) return;
    if constexpr (Resizeable<Cont>) {
        container.resize(num);
    } else {
        size_t prev_size = container.size();
        container.set_size(num);
        for (size_t i = prev_size; i < num; i++) container[i] = {};
    }
}
template <typename T, Container<T> Cont, typename... Args>
constexpr void fill_with(Cont& container, size_t num, Args... args)
requires MoveAssignable<T> && Constructible<T, Args...>
{
    if (container.size() >= num) return;
    size_t prev_size = container.size();
    container.set_size(num);
    for (size_t i = prev_size; i < num; i++) container[i] = move(T{ args... });
}
template <Numeric T>
constexpr T clamp(T val, T bottom, T top) {
    if (val <= top && val >= bottom)
        return val;
    else if (val > top)
        return top;
    else
        return bottom;
}
}    // namespace ARLib

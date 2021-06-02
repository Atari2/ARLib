#pragma once
#include "Concepts.h"
#include "Iterator.h"

namespace ARLib {
	template <IteratorConcept Iter> requires MoreComparable<typename Iter::Type>
	Iter max(Iter begin, Iter end) {
		if (begin == end) return begin;
		Iter value{ begin };
		for (; begin != end; begin++)
			if (*begin > *value) value = begin;
		return value;
	}

	template <IteratorConcept Iter> requires LessComparable<typename Iter::Type>
	Iter min(Iter begin, Iter end) {
		if (begin == end) return begin;
		Iter value{ begin };
		for (; begin != end; begin++)
			if (*begin < *value) value = begin;
		return value;
	}

	template <IteratorConcept Iter, typename Functor>
	auto sum(Iter begin, Iter end, Functor&& func) {
		if (begin == end) return InvokeResultT<Functor, decltype(*begin)>{};
		auto total = func(*begin);
		begin++;
		for (; begin != end; begin++)
			total += func(*begin);
		return total;
	}

	template <typename C> requires Iterable<C>
	auto max(const C& cont) {
		return max(cont.begin(), cont.end());
	}

	template <typename C> requires Iterable<C>
	auto min(const C& cont) {
		return min(cont.begin(), cont.end());
	}

	template <typename C, typename Functor> requires Iterable<C>
	auto sum(const C& cont, Functor&& func) {
		return sum(cont.begin(), cont.end(), Forward<Functor>(func));
	}

	// follows very naive quicksort implementation
	template <IteratorConcept Iter> requires MoreComparable<typename Iter::Type> && LessComparable<typename Iter::Type>
	Iter partition(Iter lo, Iter hi) {
		auto pivot = lo + ((hi - lo) / 2);
		auto i = lo - 1;
		auto j = hi + 1;
		for (;;) {
			do {
				i++;
			} while (*i < *pivot);
			do {
				j--;
			} while (*j > *pivot);
			if (i > j || i == j) {
				return j;
			}
			auto item = *i;
			*i = *j;
			*j = item;
		}
	}
	
	template <IteratorConcept Iter> requires MoreComparable<typename Iter::Type>&& LessComparable<typename Iter::Type>
	void quicksort_internal(Iter lo, Iter hi) {
		if (lo < hi) {
			auto p = partition(lo, hi);
			quicksort_internal(lo, p);
			quicksort_internal(p + 1, hi);
		}
	}

	// in-place sorting
	template <typename C> requires Iterable<C>
	void sort(const C& cont) {
		quicksort_internal(cont.begin(), cont.end() - 1);
	}

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

}

using ARLib::max;
using ARLib::min;
using ARLib::sort;
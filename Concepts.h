#pragma once
#include "TypeTraits.h"
#include "Iterator.h"
#include "Types.h"
#include "Badge.h"

// this is a hacky fix to an issue that can lead to the Hashable concept constraint not finding the appropriate function
// so if it wasn't included yet, include it
#ifndef HASHBASE_INCLUDE		
#include "HashBase.h"
#endif

namespace ARLib {
	namespace detail {
		template <class T, class U>
		concept SameHelper = IsSameV<T, U>;
	}

	template< class T, class U >
	concept SameAs = detail::SameHelper<T, U> && detail::SameHelper<U, T>;

	template< class Derived, class Base >
	concept DerivedFrom =
		BaseOfV<Base, Derived> &&
		ConvertibleV<const volatile Derived*, const volatile Base*>;

	template <class From, class To>
	concept ConvertibleTo =
		ConvertibleV<From, To> &&
		requires(AddRvalueReferenceT<From>(&f)()) {
		static_cast<To>(f());
	};

	// constructible concepts

	template <typename Cls, typename... Args>
	concept Constructible = requires(Args... args) {
		{ Cls{ args... } } -> SameAs<Cls>;
	};
	template <typename Cls>
	concept DefaultConstructible = requires() {
		{ Cls{} } -> SameAs<Cls>;
	};
	template <typename Cls>
	concept MoveConstructible = requires(Cls&& a) {
		{ Cls{ Forward<Cls>(a) } } -> SameAs<Cls>;
	};
	template <typename Cls>
	concept CopyConstructible = requires(const Cls& a) {
		{ Cls{ a } } -> SameAs<Cls>;
	};

	template <typename Cls>
	concept TriviallyConstructible = requires { Supports<TriviallyConstructibleV<Cls>>::value; };
	template <typename Cls>
	concept TriviallyDefaultConstructible = requires { Supports<TriviallyDefaultConstructibleV<Cls>>::value;  };
	template <typename Cls>
	concept TriviallyMoveConstructible = requires { Supports<TriviallyMoveConstructibleV<Cls>>::value; };
	template <typename Cls>
	concept TriviallyCopyConstructible = requires { Supports<TriviallyCopyConstructibleV<Cls>>::value; };

	template <typename Cls>
	concept NothrowConstructible = requires { Supports<NothrowConstructibleV<Cls>>::value; };
	template <typename Cls>
	concept NothrowDefaultConstructible = requires { Supports<NothrowDefaultConstructibleV<Cls>>::value;  };
	template <typename Cls>
	concept NothrowMoveConstructible = requires { Supports<NothrowMoveConstructibleV<Cls>>::value; };
	template <typename Cls>
	concept NothrowCopyConstructible = requires { Supports<NothrowCopyConstructibleV<Cls>>::value; };

	// assignable concepts
	template <typename Cls>
	concept Assignable = requires { Supports<AssignableV<Cls, Cls>>::value; };
	template <typename Cls>
	concept MoveAssignable = requires { Supports<MoveAssignableV<Cls>>::value; };
	template <typename Cls>
	concept CopyAssignable = requires { Supports<CopyAssignableV<Cls>>::value; };

	template <typename Cls>
	concept NothrowAssignable = requires { Supports<NothrowAssignableV<Cls, Cls>>::value; };
	template <typename Cls>
	concept NothrowMoveAssignable = requires { Supports<NothrowMoveAssignableV<Cls>>::value; };
	template <typename Cls>
	concept NothrowCopyAssignable = requires { Supports<NothrowCopyAssignableV<Cls>>::value; };

	template <typename Cls>
	concept TriviallyAssignable = requires { Supports<TriviallyAssignableV<Cls, Cls>>::value; };
	template <typename Cls>
	concept TriviallyMoveAssignable = requires { Supports<TriviallyMoveAssignableV<Cls>>::value; };
	template <typename Cls>
	concept TriviallyCopyAssignable = requires { Supports<TriviallyCopyAssignableV<Cls>>::value; };

	template <typename T>
	concept Incrementable = requires(T a) {
		{ ++a };
		{ a++ };
	};

	template <typename T>
	concept Decrementable = requires(T a) {
		{ --a };
		{ a-- };
	};

	template <typename T>
	concept Dereferencable = requires(T a) {
		{ *a };
	};

	template <typename T>
	concept IteratorConcept = Incrementable<T> && Decrementable<T> && Dereferencable<T>;


	template <typename T>
	concept EqualityComparable = requires (T a, T b) {
		{ a == b } -> ConvertibleTo<bool>;
		{ a != b} -> ConvertibleTo<bool>;
	};

	template<typename T>
	concept Hashable = requires(const T & a) {
		{ Hash<T>{}(a) } -> SameAs<size_t>;
	} && EqualityComparable<T>;


	template <typename T>
	concept LessComparable = requires (T a, T b) {
		{ a < b } -> ConvertibleTo<bool>;
	};

	template <typename T>
	concept MoreComparable = requires (T a, T b) {
		{ a > b } -> ConvertibleTo<bool>;
	};

	template <typename T>
	concept Orderable = requires (T a, T b) {
		{ a <=> b } -> ConvertibleTo<bool>;
	};

	template <typename T>
	concept Iterable = requires (T a) {
		{ a.begin() };
		{ a.end() };
	};

	template <typename T>
	concept Stringable = requires(T a) {
		{ a.to_string() };
	};

	template <typename T>
	concept Badgeable = requires(T a) {
		{ Badge<T>{} } -> SameAs<Badge<T>>;
	};
}
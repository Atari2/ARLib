#pragma once
#include "Badge.h"
#include "TypeTraits.h"
#include "Types.h"

// this is a hacky fix to an issue that can lead to the Hashable concept constraint not finding the appropriate function
// so if it wasn't included yet, include it
#ifndef HASHBASE_INCLUDE
#include "HashBase.h"
#endif

namespace ARLib {
    // FWD declare for Orderable<T>
    class Ordering;

    namespace detail {
        template <class T, class U>
        concept SameHelper = IsSameV<T, U>;

        template <class T, class U>
        concept SameHelperCvRef = IsSameCvRefV<T, U>;
    } // namespace detail

    template <class T, class U>
    concept SameAs = detail::SameHelper<T, U> && detail::SameHelper<U, T>;

    template <class T, class U>
    concept SameAsCvRef = detail::SameHelperCvRef<T, U> && detail::SameHelperCvRef<U, T>;

    template <class Derived, class Base>
    concept DerivedFrom = BaseOfV<Base, Derived> && ConvertibleV<const volatile Derived*, const volatile Base*>;

    template <class From, class To>
    concept ConvertibleTo = ConvertibleV<From, To> && requires(AddRvalueReferenceT<From> (&f)()) {
        static_cast<To>(f());
    };

    // constructible concepts

    template <typename Cls, typename... Args>
    concept Constructible = requires(Args... args) {
        {
            Cls { args... }
            } -> SameAs<Cls>;
    };
    template <typename Cls>
    concept DefaultConstructible = requires() {
        {
            Cls {}
            } -> SameAs<Cls>;
    };
    template <typename Cls>
    concept MoveConstructible = requires(Cls&& a) {
        {
            Cls { Forward<Cls>(a) }
            } -> SameAs<Cls>;
    };
    template <typename Cls>
    concept CopyConstructible = requires(const Cls& a) {
        {
            Cls { a }
            } -> SameAs<Cls>;
    };

    template <typename Cls>
    concept TriviallyConstructible = requires {
        Supports<TriviallyConstructibleV<Cls>>::value;
    };
    template <typename Cls>
    concept TriviallyDefaultConstructible = requires {
        Supports<TriviallyDefaultConstructibleV<Cls>>::value;
    };
    template <typename Cls>
    concept TriviallyMoveConstructible = requires {
        Supports<TriviallyMoveConstructibleV<Cls>>::value;
    };
    template <typename Cls>
    concept TriviallyCopyConstructible = requires {
        Supports<TriviallyCopyConstructibleV<Cls>>::value;
    };

    template <typename Cls>
    concept NothrowConstructible = requires {
        Supports<NothrowConstructibleV<Cls>>::value;
    };
    template <typename Cls>
    concept NothrowDefaultConstructible = requires {
        Supports<NothrowDefaultConstructibleV<Cls>>::value;
    };
    template <typename Cls>
    concept NothrowMoveConstructible = requires {
        Supports<NothrowMoveConstructibleV<Cls>>::value;
    };
    template <typename Cls>
    concept NothrowCopyConstructible = requires {
        Supports<NothrowCopyConstructibleV<Cls>>::value;
    };

    template <typename Cls>
    concept Trivial = requires {
        Supports<IsTrivialV<Cls>>::value;
    };

    // assignable concepts
    template <typename Cls>
    concept Assignable = requires {
        Supports<AssignableV<Cls, Cls>>::value;
    };
    template <typename Cls>
    concept MoveAssignable = requires {
        Supports<MoveAssignableV<Cls>>::value;
    };
    template <typename Cls>
    concept CopyAssignable = requires {
        Supports<CopyAssignableV<Cls>>::value;
    };

    template <typename Cls>
    concept NothrowAssignable = requires {
        Supports<NothrowAssignableV<Cls, Cls>>::value;
    };
    template <typename Cls>
    concept NothrowMoveAssignable = requires {
        Supports<NothrowMoveAssignableV<Cls>>::value;
    };
    template <typename Cls>
    concept NothrowCopyAssignable = requires {
        Supports<NothrowCopyAssignableV<Cls>>::value;
    };

    template <typename Cls>
    concept TriviallyAssignable = requires {
        Supports<TriviallyAssignableV<Cls, Cls>>::value;
    };
    template <typename Cls>
    concept TriviallyMoveAssignable = requires {
        Supports<TriviallyMoveAssignableV<Cls>>::value;
    };
    template <typename Cls>
    concept TriviallyCopyAssignable = requires {
        Supports<TriviallyCopyAssignableV<Cls>>::value;
    };

    template <typename T>
    concept Incrementable = requires(T a) {
        {++a};
        {a++};
    };

    template <typename T>
    concept Decrementable = requires(T a) {
        {--a};
        {a--};
    };

    template <typename T>
    concept Dereferencable = requires(T a) {
        {*a};
    };

    template <typename T>
    concept IteratorConcept = Incrementable<T> && Decrementable<T> && Dereferencable<T>;

    template <typename T>
    concept EqualityComparable = requires(T a, T b) {
        { a == b } -> ConvertibleTo<bool>;
        { a != b } -> ConvertibleTo<bool>;
    };

    template <typename T, typename C>
    concept EqualityComparableWith = requires(T a, C b) {
        { a == b } -> ConvertibleTo<bool>;
        { a != b } -> ConvertibleTo<bool>;
    };

    template <typename T>
    concept Hashable = requires(const T& a) {
        { Hash<T>{}(a) } -> SameAs<size_t>;
    }
    &&EqualityComparable<T>;

    template <typename T>
    concept LessComparable = requires(T a, T b) {
        { a < b } -> ConvertibleTo<bool>;
    };

    template <typename T>
    concept MoreComparable = requires(T a, T b) {
        { a > b } -> ConvertibleTo<bool>;
    };

    template <typename T>
    concept Orderable = requires(T a, T b) {
        { a <=> b } -> ConvertibleTo<Ordering>;
    };

    template <typename T>
    concept Iterable = requires(T a) {
        {a.begin()};
        {a.end()};
    };

    template <typename T>
    concept EnumerableC = Iterable<T> && requires(T a) {
        {a.size()};
    };

    template <typename T>
    concept IterCanSubtractForSize = requires(T a, T b) {
        { a - b } -> SameAs<size_t>;
    };

    template <typename T>
    concept Stringable = requires(T a) {
        {a.to_string()};
    };

    template <typename T>
    concept Badgeable = requires(T a) {
        {
            Badge<T> {}
            } -> SameAs<Badge<T>>;
    };

    template <typename C>
    concept Resizeable = requires(C a) {
        {a.resize()};
    };

    template <typename C>
    concept Reservable = requires(C a) {
        {a.reserve(0ull)};
    };

    template <typename Callable, typename... Args>
    concept CallableWith = requires(Callable func, Args... args) {
        { func(args...) } -> SameAs<ResultOfT<Callable(Args...)>>;
    };

    template <typename Cls, typename F, typename Res, typename... Args>
    concept CallMembFnImpl = requires {
        { (declval<F>())(declval<Args>()...) } -> SameAs<Res>;
    };

    template <typename Cls, typename F, typename Res, typename MaybeClsPtr, typename... Args>
    struct CallMembFnImplStruct {
        static constexpr bool is_mbm_fn = IsSameV<MaybeClsPtr, Cls*> ?
                                          CallMembFnImpl<Cls, F, Res, Args...> :
                                          CallMembFnImpl<Cls, F, Res, MaybeClsPtr, Args...>;
    };

    template <typename Cls, typename F, typename Res, typename... Args>
    concept CallMembFn = CallMembFnImplStruct<Cls, F, Res, Args...>::is_mbm_fn;

    template <typename Callable, typename Res, typename... Args>
    concept CallableWithRes = requires(Callable func, Args... args) {
        { func(args...) } -> SameAs<Res>;
    }
    || CallMembFn<MembFnCls<Callable>, MembFnFn<Callable>, Res, Args...>;

    template <typename T>
    concept Swappable = requires(T a, T b) {
        { swap(a, b) } -> SameAs<void>;
    };

    template <typename Cont, typename T>
    concept Container = requires(Cont container) {
        { container[0ull] } -> SameAs<T&>;
        { container.size() } -> SameAs<size_t>;
        {container.set_size(0ull)};
    }
    &&Reservable<Cont>;

    template <typename Cont>
    concept CanKnowSize = requires(Cont container) {
        { container.size() } -> SameAs<size_t>;
    };

    template <typename Cont, typename T>
    concept Pushable = requires(Cont container, T elem) {
        { container.push_back(elem) } -> SameAs<void>;
    };

    template <typename T>
    concept Integral = IsIntegralV<T>;

    template <typename T>
    concept FloatingPoint = IsFloatingPointV<T>;

    template <typename T>
    concept Numeric = IsNumericV<T>;

    template <typename T>
    concept Sized = IsSizeV<T>;

    template <typename T>
    concept Enum = IsEnumV<T>;

    template <typename T>
    concept IsSigned = IsIntegralV<T> && requires(const T& val) {
        {static_cast<T>(-1) > 0 == true};
    };
} // namespace ARLib

#pragma once
#include "Badge.hpp"
#include "TypeTraits.hpp"
#include "Types.hpp"
#include "Invoke.hpp"

// this is a hacky fix to an issue that can lead to the Hashable concept constraint not finding the appropriate function
// so if it wasn't included yet, include it
#ifndef HASHBASE_INCLUDE
    #include "HashBase.hpp"
#endif
namespace ARLib {
// FWD declare for Orderable<T>
class Ordering;
namespace detail {
    template <class T, class U>
    concept SameHelper = IsSameV<T, U>;

    template <class T, class U>
    concept SameHelperCvRef = IsSameCvRefV<T, U>;
}    // namespace detail

template <class T, class U>
concept SameAs = detail::SameHelper<T, U> && detail::SameHelper<U, T>;

template <class T, class U>
concept SameAsCvRef = detail::SameHelperCvRef<T, U> && detail::SameHelperCvRef<U, T>;

template <class Derived, class Base>
concept DerivedFrom = BaseOfV<Base, Derived> && ConvertibleV<const volatile Derived*, const volatile Base*>;

template <class From, class To>
concept ConvertibleTo = ConvertibleV<From, To> && requires(AddRvalueReferenceT<From> (&f)()) { static_cast<To>(f()); };

// constructible concepts

template <typename Cls, typename... Args>
concept Constructible = requires(Args&&... args) {
    { Cls{ ForwardTrait<Args>(args)... } } -> SameAsCvRef<Cls>;
};
template <typename Cls>
concept DefaultConstructible = requires() {
    { Cls{} } -> SameAsCvRef<Cls>;
};
template <typename Cls>
concept MoveConstructible = requires(Cls&& a) {
    { Cls{ ForwardTrait<Cls>(a) } } -> SameAsCvRef<Cls>;
};
template <typename Cls>
concept CopyConstructible = requires(const Cls& a) {
    { Cls{ a } } -> SameAsCvRef<Cls>;
};

template <typename Cls>
concept TriviallyConstructible = requires { Supports<TriviallyConstructibleV<Cls>>::value; };
template <typename Cls>
concept TriviallyDefaultConstructible = requires { Supports<TriviallyDefaultConstructibleV<Cls>>::value; };
template <typename Cls>
concept TriviallyMoveConstructible = requires { Supports<TriviallyMoveConstructibleV<Cls>>::value; };
template <typename Cls>
concept TriviallyCopyConstructible = requires { Supports<TriviallyCopyConstructibleV<Cls>>::value; };

template <typename Cls>
concept NothrowConstructible = requires { Supports<NothrowConstructibleV<Cls>>::value; };
template <typename Cls>
concept NothrowDefaultConstructible = requires { Supports<NothrowDefaultConstructibleV<Cls>>::value; };
template <typename Cls>
concept NothrowMoveConstructible = requires { Supports<NothrowMoveConstructibleV<Cls>>::value; };
template <typename Cls>
concept NothrowCopyConstructible = requires { Supports<NothrowCopyConstructibleV<Cls>>::value; };

template <typename Cls>
concept Trivial = requires { Supports<IsTrivialV<Cls>>::value; };

// assignable concepts
template <typename Cls>
concept Assignable = requires { Supports<AssignableV<Cls, Cls>>::value; };
template <typename Cls>
concept MoveAssignable = requires { Supports<MoveAssignableV<Cls>>::value; };
template <typename Cls>
concept CopyAssignable = requires { Supports<CopyAssignableV<Cls>>::value; };

template <typename Cls, typename Other = Cls>
concept NothrowAssignable = requires { Supports<NothrowAssignableV<Cls, Other>>::value; };
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
concept ForwardIterator = Incrementable<T> && Dereferencable<T>;

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

template <typename T, typename HashCls = Hash<RemoveCvRefT<T>>>
concept Hashable = requires(const T& a) {
    { HashCls{}(a) } -> SameAs<size_t>;
} && EqualityComparable<T>;

template <typename T>
concept LessComparable = requires(T a, T b) {
    { a < b } -> ConvertibleTo<bool>;
};

template <typename T>
concept MoreComparable = requires(T a, T b) {
    { a > b } -> ConvertibleTo<bool>;
};

template <typename T>
concept Orderable = requires(const T& a, const T& b) {
    { a <=> b } -> ConvertibleTo<Ordering>;
};

template <typename T>
concept Iterable = requires(T a) {
    { a.begin() };
    { a.end() };
};

template <typename T>
concept IterCanSubtractForSize = requires(T a, T b) {
    { a - b } -> SameAs<size_t>;
};
template <typename T>
concept EnumerableC = Iterable<T> && (requires(T a) {
                          { a.size() };
                      } || IterCanSubtractForSize<T>);

template <typename T>
concept Stringable = requires(T a) {
    { a.to_string() };
};

template <typename T>
concept Badgeable = requires(T a) {
    { Badge<T>{} } -> SameAs<Badge<T>>;
};

template <typename C>
concept Resizeable = requires(C a) {
    { a.resize() };
};

template <typename C>
concept Reservable = requires(C a) {
    { a.reserve(0ull) };
};

template <typename Callable, typename... Args>
concept CallableWith = requires(Callable func, Args... args) {
    { invoke(func, args...) } -> SameAs<ResultOfT<Callable(Args...)>>;
};

template <typename Cls, typename F, typename Res, typename... Args>
concept CallMembFnImpl = requires {
    { (declval<F>())(declval<Args>()...) } -> SameAs<Res>;
};
template <typename Cls, typename F, typename Res, typename MaybeClsPtr, typename... Args>
struct CallMembFnImplStruct {
    constexpr static bool is_mbm_fn = IsSameV<MaybeClsPtr, Cls*> ? CallMembFnImpl<Cls, F, Res, Args...> :
                                                                   CallMembFnImpl<Cls, F, Res, MaybeClsPtr, Args...>;
};
template <typename Cls, typename F, typename Res, typename... Args>
concept CallMembFn = CallMembFnImplStruct<Cls, F, Res, Args...>::is_mbm_fn;

template <typename Callable, typename Res, typename... Args>
concept CallableWithRes = requires(Callable func, Args... args) {
    { invoke_r<Res>(func, args...) } -> SameAs<Res>;
} || CallMembFn<MembFnCls<Callable>, MembFnFn<Callable>, Res, Args...>;

template <typename T>
concept Swappable = requires(T a, T b) {
    { swap(a, b) } -> SameAs<void>;
};

template <typename T>
concept NonVoid = !IsVoidV<T>;

template <typename Cont>
concept Container = requires(Cont container) {
    { container[0ull] } -> NonVoid;
    { container.size() } -> SameAs<size_t>;
    { container.set_size(0ull) };
} && Reservable<Cont>;

template <typename Cont>
concept CanKnowSize = requires(Cont container) {
    { container.size() } -> SameAs<size_t>;
};

template <typename Cont, typename T>
concept Pushable = requires(Cont container, T elem) {
    { container.push_back(ForwardTrait<T>(elem)) } -> SameAs<void>;
} || requires(Cont container, T elem) {
    { container.append(ForwardTrait<T>(elem)) } -> SameAs<void>;
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
concept IsSigned = IsIntegralV<T> && static_cast<T>(-1) < 0;

template <typename T>
concept SignedIntegral = Integral<T> && IsSigned<T>;

template <typename T>
concept UnsignedIntegral = Integral<T> && (!IsSigned<T>);

}    // namespace ARLib

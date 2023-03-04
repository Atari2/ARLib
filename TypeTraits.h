#pragma once
#include "BaseTraits.h"
/*
    the very basic type traits and some of the more complex ones (such as std::invoke_result)
    are derived from MSVC's STL implementation https://github.com/microsoft/STL
    which operates under the Apache License with LLVM exception, with notice included here:

    Copyright (c) Microsoft Corporation.
    SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
*/

namespace ARLib {

template <class>
constexpr inline bool AlwaysFalse = false;
template <class T>
compiler_intrinsic constexpr inline T&& ForwardTrait(typename RemoveReference<T>::type& t) noexcept {
    return static_cast<T&&>(t);
}
template <class T>
compiler_intrinsic constexpr inline T&& ForwardTrait(typename RemoveReference<T>::type&& t) noexcept {
    return static_cast<T&&>(t);
}
template <typename P>
struct Not : public BoolConstant<!bool(P::value)> {};

template <typename...>
struct Or;
template <>
struct Or<> : public FalseType {};
template <typename B1>
struct Or<B1> : public B1 {};
template <typename B1, typename B2>
struct Or<B1, B2> : public Conditional<B1::value, B1, B2>::type {};
template <typename B1, typename B2, typename B3, typename... Bn>
struct Or<B1, B2, B3, Bn...> : public Conditional<B1::value, B1, Or<B2, B3, Bn...>>::type {};
template <typename T>
struct IsObject : public Not<Or<IsFunction<T>, IsReference<T>, IsVoid<T>>>::type {};

template <typename U, bool IsArray = IsArray<U>::value, bool IsFunction = IsFunction<U>::value>
struct DecaySelector;
template <typename U>
struct DecaySelector<U, false, false> {
    typedef typename RemoveCv<U>::type type;
};
template <typename U>
struct DecaySelector<U, true, false> {
    typedef typename RemoveExtent<U>::type* type;
};
template <typename U>
struct DecaySelector<U, false, true> {
    typedef typename AddPointer<U>::type type;
};
template <typename T>
class Decay {
    typedef typename RemoveReference<T>::type remove_type;

    public:
    typedef typename DecaySelector<remove_type>::type type;
};
template <typename T>
using DecayT = typename Decay<T>::type;

template <typename T>
using RemoveCvRefT = typename RemoveCv<typename RemoveReference<T>::type>::type;
template <typename T, typename U = RemoveCvRefT<T>>
struct InvUnwrap {
    using type = T;
};
template <typename T, typename U>
struct InvUnwrap<T, detail::ReferenceWrapper<U>> {
    using type = U&;
};
template <typename T, typename U = RemoveCvRefT<T>>
using InvUnwrapT = typename InvUnwrap<T, U>::type;
template <typename>
struct IsMemberObjectPointerHelper : public FalseType {};
template <typename T, typename C>
struct IsMemberObjectPointerHelper<T C::*> : public Not<IsFunction<T>>::type {};
template <typename T>
struct IsMemberObjectPointer : public IsMemberObjectPointerHelper<typename RemoveCv<T>::type>::type {};
template <typename>
struct IsMemberFunctionPointerHelper : public FalseType {};
template <typename T, typename C>
struct IsMemberFunctionPointerHelper<T C::*> : public IsFunction<T>::type {};
template <typename T>
struct IsMemberFunctionPointer : public IsMemberFunctionPointerHelper<typename RemoveCv<T>::type>::type {};
template <typename T>
constexpr bool IsMemberFunctionPointerV = IsMemberFunctionPointer<T>::value;
template <typename C>
struct MembFnUnwrap {};
template <typename T, typename C>
struct MembFnUnwrap<T C::*> {
    using cls = C;
    using fn  = T;
};
template <typename T>
using MembFnCls = typename MembFnUnwrap<RemoveReferenceT<T>>::cls;

template <typename T>
using MembFnFn = typename MembFnUnwrap<RemoveReferenceT<T>>::fn;
template <typename Ptr>
struct PointerTraits {
    private:
    template <typename T>
    using element_type_internal = typename T::element_type;

    template <typename T>
    using difference_type_internal = typename T::difference_type;
    template <typename T, typename U, typename = void>
    struct rebind_internal : ReplaceFirstArg<T, U> {};
    template <typename T, typename U>
    struct rebind_internal<T, U, VoidT<typename T::template rebind<U>>> {
        using type = typename T::template rebind<U>;
    };

    public:
    /// The pointer type.
    using pointer = Ptr;

    /// The type pointed to.
    using element_type = DetectedOrT<GetFirstArgT<Ptr>, element_type_internal, Ptr>;

    /// The type used to represent the difference between two pointers.
    using difference_type = DetectedOrT<ptrdiff_t, difference_type_internal, Ptr>;

    /// A pointer to a different type.
    template <typename U>
    using rebind = typename rebind_internal<Ptr, U>::type;
    constexpr static Ptr pointer_to(MakeNotVoid<element_type>& e) { return Ptr::pointer_to(e); }
    static_assert(
    !IsSame<element_type, Undefined>::value, "pointer type defines element_type or is like SomePointer<T, Args>"
    );
};
template <typename T>
struct PointerTraits<T*> {
    /// The pointer type
    typedef T* pointer;
    /// The type pointed to
    typedef T element_type;
    /// Type used to represent the difference between two pointers
    typedef ptrdiff_t difference_type;

    template <typename U>
    using rebind = U*;
    constexpr static pointer pointer_to(MakeNotVoid<element_type>& r) noexcept { return ARLib::addressof(r); }
};
// isclass
template <class T>
struct IsClass : decltype(detail::Test<T>(nullptr)) {};
// convertible
template <class From, class To>
struct Convertible :
    IntegralConstant<
    bool,
    (
    decltype(detail::TestReturnable<To>(0))::value && decltype(detail::TestImplicitlyConvertible<From, To>(0))::value
    ) ||
    (IsVoid<From>::value && IsVoid<To>::value)> {};
template <class From, class To>
struct NothrowConvertible : Conjunction<IsVoid<From>, IsVoid<To>> {};
// baseof
template <typename Base, typename Derived>
struct BaseOf :
    IntegralConstant<
    bool,
    IsClass<Base>::value && IsClass<Derived>::value&& decltype(detail::TestPreIsBaseOf<Base, Derived>(0))::value> {};
// constructible in various ways -> todo in concepts
template <class T>
struct CopyConstructibleImpl : ConstructibleImpl<T, typename AddLvalueReference<typename AddConst<T>::type>::type> {};
template <class T>
struct TriviallyCopyConstructibleImpl :
    TriviallyConstructibleImpl<T, typename AddLvalueReference<typename AddConst<T>::type>::type> {};
template <class T>
struct NothrowCopyConstructibleImpl :
    NothrowConstructibleImpl<T, typename AddLvalueReference<typename AddConst<T>::type>::type> {};
template <class T>
struct MoveConstructibleImpl : ConstructibleImpl<T, typename AddRvalueReference<T>::type> {};
template <class T>
struct TriviallyMoveConstructibleImpl : TriviallyConstructibleImpl<T, typename AddRvalueReference<T>::type> {};
template <class T>
struct NothrowMoveConstructibleImpl : NothrowConstructibleImpl<T, typename AddRvalueReference<T>::type> {};
// assignable in various ways -> todo in concepts
template <class T>
struct CopyAssignableImpl :
    AssignableImpl<typename AddLvalueReference<T>::type, typename AddLvalueReference<const T>::type> {};
template <class T>
struct TriviallyCopyAssignableImpl :
    TriviallyAssignableImpl<typename AddLvalueReference<T>::type, typename AddLvalueReference<const T>::type> {};
template <class T>
struct NothrowCopyAssignableImpl :
    NothrowAssignableImpl<typename AddLvalueReference<T>::type, typename AddLvalueReference<const T>::type> {};
template <class T>
struct MoveAssignableImpl :
    AssignableImpl<typename AddLvalueReference<T>::type, typename AddRvalueReference<T>::type> {};
template <class T>
struct TriviallyMoveAssignableImpl :
    TriviallyAssignableImpl<typename AddLvalueReference<T>::type, typename AddRvalueReference<T>::type> {};
template <class T>
struct NothrowMoveAssignableImpl :
    NothrowAssignableImpl<typename AddLvalueReference<T>::type, typename AddRvalueReference<T>::type> {};
template <class T>
struct DefaultConstructibleImpl : ConstructibleImpl<T> {};
template <class T>
struct TriviallyDefaultConstructibleImpl : TriviallyConstructibleImpl<T> {};
template <class T>
struct NothrowDefaultConstructibleImpl : NothrowConstructibleImpl<T> {};

// helpers
template <class T>
using AddLvalueReferenceT = typename AddLvalueReference<T>::type;

template <class T>
using AddRvalueReferenceT = typename AddRvalueReference<T>::type;
template <bool>
struct Select {
    template <class T1, class>
    using Apply = T1;
};
template <>
struct Select<false> {
    template <class, class T2>
    using Apply = T2;
};

template <size_t>
struct MakeUnsignedHelper;
template <>
struct MakeUnsignedHelper<1> {
    template <class>
    using Apply = uint8_t;
};
template <>
struct MakeUnsignedHelper<2> {
    template <class>
    using Apply = uint16_t;
};
template <>
struct MakeUnsignedHelper<4> {
    template <class>
    using Apply = uint32_t;
};
template <>
struct MakeUnsignedHelper<8> {
    template <class>
    using Apply = uint64_t;
};

template <class T>
using MakeUnsignedBase = typename MakeUnsignedHelper<sizeof(T)>::template Apply<T>;
template <class T>
struct MakeUnsigned {
    static_assert(IsNonboolIntegral<T> || IsEnumV<T>, "make_unsigned<T> requires a non bool integral type.");

    using type = typename RemoveCv<T>::template Apply<MakeUnsignedBase>;
};

template <class T>
using MakeUnsignedT = typename MakeUnsigned<T>::type;
namespace detail {
    template <class T>
    struct InvokeImpl {
        template <class F, class... Args>
        static auto call(F&& f, Args&&... args) -> decltype(ForwardTrait<F>(f)(ForwardTrait<Args>(args)...));
    };
    template <class B, class MT>
    struct InvokeImpl<MT B::*> {
        template <class T, class Td = typename Decay<T>::type, class = typename EnableIf<BaseOf<B, Td>::value>::type>
        static auto get(T&& t) -> T&&;

        template <
        class T, class Td = typename Decay<T>::type, class = typename EnableIf<ReferenceWrapper<Td>::value>::type>
        static auto get(T&& t) -> decltype(t.get());

        template <
        class T, class Td = typename Decay<T>::type, class = typename EnableIf<!BaseOf<B, Td>::value>::type,
        class = typename EnableIf<!ReferenceWrapper<Td>::value>::type>
        static auto get(T&& t) -> decltype(*ForwardTrait<T>(t));

        template <class T, class... Args, class MT1, class = typename EnableIf<IsFunction<MT1>::value>::type>
        static auto call(MT1 B::*pmf, T&& t, Args&&... args)
        -> decltype((InvokeImpl::get(ForwardTrait<T>(t)).*pmf)(ForwardTrait<Args>(args)...));

        template <class T>
        static auto call(MT B::*pmd, T&& t) -> decltype(InvokeImpl::get(ForwardTrait<T>(t)).*pmd);
    };

    template <class F, class... Args, class Fd = typename Decay<F>::type>
    auto INVOKE(F&& f, Args&&... args)
    -> decltype(InvokeImpl<Fd>::call(ForwardTrait<F>(f), ForwardTrait<Args>(args)...));
    template <typename AlwaysVoid, typename, typename...>
    struct InvokeResult {};
    template <typename F, typename... Args>
    struct InvokeResult<decltype(void(detail::INVOKE(declval<F>(), declval<Args>()...))), F, Args...> {
        using type = decltype(detail::INVOKE(declval<F>(), declval<Args>()...));
    };
    template <typename T>
    struct SuccessType {
        typedef T type;
    };
    struct FailureType {};
    struct InvokeOther {};
    struct InvokeMemfunDeref {};
    struct InvokeMemfunRef {};
    struct InvokeMemobjDeref {};
    struct InvokeMemobjRef {};
    template <typename T, typename Tag>
    struct ResultOfSuccess : SuccessType<T> {
        using invoke_type = Tag;
    };
    struct ResultOfMemFunRefImpl {
        template <typename F, typename T, typename... Args>
        static ResultOfSuccess<decltype((declval<T>().*declval<F>())(declval<Args>()...)), InvokeMemfunRef> s_test(int);

        template <typename...>
        static FailureType s_test(...);
    };
    template <typename MemPtr, typename Arg, typename... Args>
    struct ResultOfMemFunRef : private ResultOfMemFunRefImpl {
        typedef decltype(s_test<MemPtr, Arg, Args...>(0)) type;
    };
    struct ResultOfMemFunDerefImpl {
        template <typename F, typename T, typename... Args>
        static ResultOfSuccess<decltype(((*declval<T>()).*declval<F>())(declval<Args>()...)), InvokeMemfunDeref>
        s_test(int);

        template <typename...>
        static FailureType s_test(...);
    };
    template <typename MemPtr, typename Arg, typename... Args>
    struct ResultOfMemFunDeref : private ResultOfMemFunDerefImpl {
        typedef decltype(s_test<MemPtr, Arg, Args...>(0)) type;
    };
    struct ResultOfMemObjRefImpl {
        template <typename F, typename T>
        static ResultOfSuccess<decltype(declval<T>().*declval<F>()), InvokeMemobjRef> s_test(int);

        template <typename, typename>
        static FailureType s_test(...);
    };
    template <typename MemPtr, typename Arg>
    struct ResultOfMemObjRef : private ResultOfMemObjRefImpl {
        typedef decltype(s_test<MemPtr, Arg>(0)) type;
    };
    struct ResultOfMemObjDerefImpl {
        template <typename F, typename T>
        static ResultOfSuccess<decltype((*declval<T>()).*declval<F>()), InvokeMemobjDeref> s_test(int);

        template <typename, typename>
        static FailureType s_test(...);
    };
    template <typename MemPtr, typename Arg>
    struct ResultOfMemObjDeref : private ResultOfMemObjDerefImpl {
        typedef decltype(s_test<MemPtr, Arg>(0)) type;
    };

    template <typename MemPtr, typename Arg>
    struct ResultOfMemObj;
    template <typename Res, typename Class, typename Arg>
    struct ResultOfMemObj<Res Class::*, Arg> {
        typedef RemoveCvRefT<Arg> Argval;
        typedef Res Class::*Memptr;
        typedef typename Conditional<
        Or<IsSame<Argval, Class>, BaseOf<Class, Argval>>::value, ResultOfMemObjRef<Memptr, Arg>,
        ResultOfMemObjDeref<Memptr, Arg>>::type::type type;
    };

    template <typename MemPtr, typename Arg, typename... Args>
    struct ResultOfMemFun;
    template <typename Res, typename Class, typename Arg, typename... Args>
    struct ResultOfMemFun<Res Class::*, Arg, Args...> {
        typedef typename RemoveReference<Arg>::type Argval;
        typedef Res Class::*MemPtr;
        typedef typename Conditional<
        BaseOf<Class, Argval>::value, ResultOfMemFunRef<MemPtr, Arg, Args...>,
        ResultOfMemFunDeref<MemPtr, Arg, Args...>>::type::type type;
    };
    template <bool, bool, typename Functor, typename... Args>
    struct ResultOfImpl {
        typedef FailureType type;
    };
    template <typename MemPtr, typename Arg>
    struct ResultOfImpl<true, false, MemPtr, Arg> :
        public ResultOfMemObj<typename Decay<MemPtr>::type, typename InvUnwrap<Arg>::type> {};
    template <typename MemPtr, typename Arg, typename... Args>
    struct ResultOfImpl<false, true, MemPtr, Arg, Args...> :
        public ResultOfMemFun<typename Decay<MemPtr>::type, typename InvUnwrap<Arg>::type, Args...> {};
    struct ResultOfOtherImpl {
        template <typename F, typename... Args>
        static ResultOfSuccess<decltype(declval<F>()(std::declval<Args>()...)), InvokeOther> s_test(int);

        template <typename...>
        static FailureType s_test(...);
    };
    template <typename Functor, typename... Args>
    struct ResultOfImpl<false, false, Functor, Args...> : private ResultOfOtherImpl {
        typedef decltype(s_test<Functor, Args...>(0)) type;
    };
    template <typename Functor, typename... Args>
    struct InvokeResultForFunc :
        public ResultOfImpl<
        IsMemberObjectPointer<typename RemoveReference<Functor>::type>::value,
        IsMemberFunctionPointer<typename RemoveReference<Functor>::type>::value, Functor, Args...>::type {};
}    // namespace detail
template <class T, bool = IsEnumV<T>>
struct UnderlyingTypeImpl {
    using type = __underlying_type(T);
};
template <typename Result, typename Ret, bool = IsVoid<Ret>::value, typename = void>
struct IsInvokableImpl : FalseType {};
template <typename Result, typename Ret>
struct IsInvokableImpl<
Result, Ret,
/* IsVoid<Ret> = */ true, VoidT<typename Result::type>> : TrueType {};
template <typename Result, typename Ret>
struct IsInvokableImpl<
Result, Ret,
/* IsVoid<Ret> = */ false, VoidT<typename Result::type>> {
    private:
    static typename Result::type s_get();

    template <typename T>
    static void s_conv(T);

    // This overload is viable if INVOKE(f, args...) can convert to _Tp.
    template <typename T, typename = decltype(s_conv<T>(s_get()))>
    static TrueType s_test(int);

    template <typename T>
    static FalseType s_test(...);

    public:
    using type = decltype(s_test<Ret>(1));
};
template <class T>
struct UnderlyingTypeImpl<T, false> {};
template <class T>
struct UnderlyingType : UnderlyingTypeImpl<T> {};    // determine underlying type for enum

template <class T>
using UnderlyingTypeT = typename UnderlyingType<T>::type;

template <class>
struct ResultOf;
template <class F, class... ArgTypes>
struct ResultOf<F(ArgTypes...)> : detail::InvokeResult<void, F, ArgTypes...> {};
template <class F, class... ArgTypes>
struct InvokeResult : detail::InvokeResult<void, F, ArgTypes...> {};

template <class T>
using ResultOfT = typename ResultOf<T>::type;

template <class F, class... ArgTypes>
using InvokeResultT = typename InvokeResult<F, ArgTypes...>::type;

using MaxAlignT = double;
template <class T>
struct AlignmentOf : IntegralConstant<size_t, alignof(T)> {};

template <class T>
constexpr inline size_t AlignmentOfV = alignof(T);
template <class T, size_t N>
union AlignType {
    T Val;
    char Pad[N];
};

template <size_t N, size_t Align, class T, bool Ok>
struct Aligned;
template <size_t N, size_t Align, class T>
struct Aligned<N, Align, T, true> {
    using type = AlignType<T, N>;
};
template <size_t N, size_t Align>
struct Aligned<N, Align, double, false> {
    static_assert(AlwaysFalse<Aligned>, "Wrong alignment for type");
    using type = AlignType<MaxAlignT, N>;
};
template <size_t N, size_t Align>
struct Aligned<N, Align, int, false> {
    using Next                 = double;
    constexpr static bool Fits = Align <= alignof(Next);
    using type                 = typename Aligned<N, Align, Next, Fits>::type;
};
template <size_t N, size_t Align>
struct Aligned<N, Align, short, false> {
    using Next                 = int;
    constexpr static bool Fits = Align <= alignof(Next);
    using type                 = typename Aligned<N, Align, Next, Fits>::type;
};
template <size_t N, size_t Align>
struct Aligned<N, Align, char, false> {
    using Next                 = short;
    constexpr static bool Fits = Align <= alignof(Next);
    using type                 = typename Aligned<N, Align, Next, Fits>::type;
};
template <size_t N, size_t Align = alignof(MaxAlignT)>
struct AlignedStorage {
    using Next                 = char;
    constexpr static bool Fits = Align <= alignof(Next);
    using type                 = typename Aligned<N, Align, Next, Fits>::type;
};
template <size_t N, size_t Align = alignof(MaxAlignT)>
using AlignedStorageT = typename AlignedStorage<N, Align>::type;

template <class T, class... Args>
constexpr inline bool ConstructibleV = ConstructibleImpl<T, Args...>::value;
template <class T, class... Args>
constexpr inline bool TriviallyConstructibleV = TriviallyConstructibleImpl<T, Args...>::value;
template <class T, class... Args>
constexpr inline bool NothrowConstructibleV = NothrowConstructibleImpl<T, Args...>::value;

template <class T, class U>
constexpr inline bool AssignableV = AssignableImpl<T, U>::value;
template <class T, class U>
constexpr inline bool TriviallyAssignableV = TriviallyAssignableImpl<T, U>::value;
template <class T, class U>
constexpr inline bool NothrowAssignableV = NothrowAssignableImpl<T, U>::value;

template <class T>
constexpr inline bool MoveAssignableV = MoveAssignableImpl<T>::value;
template <class T>
constexpr inline bool CopyAssignableV = CopyAssignableImpl<T>::value;
template <class T>
constexpr inline bool MoveConstructibleV = MoveConstructibleImpl<T>::value;
template <class T>
constexpr inline bool CopyConstructibleV = CopyConstructibleImpl<T>::value;
template <class T>
constexpr inline bool NothrowMoveAssignableV = NothrowMoveAssignableImpl<T>::value;
template <class T>
constexpr inline bool NothrowCopyAssignableV = NothrowCopyAssignableImpl<T>::value;
template <class T>
constexpr inline bool NothrowMoveConstructibleV = NothrowMoveConstructibleImpl<T>::value;
template <class T>
constexpr inline bool NothrowCopyConstructibleV = NothrowCopyConstructibleImpl<T>::value;
template <class T>
constexpr inline bool TriviallyMoveAssignableV = TriviallyMoveAssignableImpl<T>::value;
template <class T>
constexpr inline bool TriviallyCopyAssignableV = TriviallyCopyAssignableImpl<T>::value;
template <class T>
constexpr inline bool TriviallyMoveConstructibleV = TriviallyMoveConstructibleImpl<T>::value;
template <class T>
constexpr inline bool TriviallyCopyConstructibleV = TriviallyCopyConstructibleImpl<T>::value;
template <class T>
using RemoveExtentT = typename RemoveExtent<T>::type;

template <class T>
constexpr inline bool DefaultConstructibleV = DefaultConstructibleImpl<T>::value;

template <class T>
constexpr inline bool TriviallyDefaultConstructibleV = TriviallyDefaultConstructibleImpl<T>::value;

template <class T>
constexpr inline bool NothrowDefaultConstructibleV = NothrowDefaultConstructibleImpl<T>::value;

template <class Base, class Derived>
constexpr inline bool BaseOfV = BaseOf<Base, Derived>::value;

template <class T, class U>
constexpr inline bool IsSameV = IsSame<T, U>::value;

template <class T, class U>
constexpr inline bool IsSameCvRefV = IsSameCvRef<T, U>::value;

template <class T>
constexpr inline bool IsConstV = IsConst<T>::value;

template <class>
constexpr inline bool IsVolatileV = false;

template <class T>
constexpr inline bool IsVolatileV<volatile T> = true;
template <class T>
struct IsVolatile : BoolConstant<IsVolatileV<T>> {};

template <class>
constexpr inline bool IsPointerV = false;

template <class T>
constexpr inline bool IsPointerV<T*> = true;

template <class T>
constexpr inline bool IsPointerV<T* const> = true;

template <class T>
constexpr inline bool IsPointerV<T* volatile> = true;

template <class T>
constexpr inline bool IsPointerV<T* const volatile> = true;
template <class T>
struct IsPointer : BoolConstant<IsPointerV<T>> {};

template <class From, class To>
constexpr inline bool ConvertibleV = Convertible<From, To>::value;

template <class From, class To>
constexpr inline bool NothrowConvertibleV = NothrowConvertible<From, To>::value;

template <class T>
constexpr inline bool IsLvalueReferenceV = IsLvalueReference<T>::value;
template <bool B>
struct Supports {};
template <>
struct Supports<true> {
    constexpr static inline bool value = true;
};
template <bool B>
constexpr inline bool SupportsV = Supports<B>::value;
// Helper to SFINAE away emplace variadic template parameters constructors
template <class T>
struct EmplaceT {};
template <class T>
AddLvalueReferenceT<T> priv_declval() noexcept {
    static_assert(AlwaysFalse<T>, "Declval cannot be called");
}
template <typename T, typename... Types>
struct TypeTuple : TypeTuple<Types...> {
    using RealT = ConditionalT<IsLvalueReferenceV<T>, T, AddPointerT<T>>;
    RealT dummy;
    template <size_t N>
    constexpr decltype(auto) get() const {
        if constexpr (N == 0 && IsPointerV<RealT>) {
            return *dummy;
        } else if constexpr (N == 0) {
            return dummy;
        } else {
            static_assert(N < (sizeof...(Types) + 1) && N > 0);
            return static_cast<const TypeTuple<Types...>*>(this)->template get<N - 1>();
        }
    }
};
template <typename T>
struct TypeTuple<T> {
    using RealT = ConditionalT<IsLvalueReferenceV<T>, T, AddPointerT<T>>;
    RealT dummy;
    template <size_t N>
    requires(N == 0)
    constexpr decltype(auto) get() const {
        if constexpr (IsPointerV<RealT>) {
            return *dummy;
        } else {
            return dummy;
        }
    }
};
template <typename... Types>
struct TypeArray {
    using TT = TypeTuple<Types...>;
    template <size_t I>
    requires(I < sizeof...(Types))
    using At = RemoveReferenceT<decltype(priv_declval<TT>().template get<I>())>;
};
}    // namespace ARLib

#pragma once
#include "Concepts.h"
#include "TypeTraits.h"
#include "Utility.h"
namespace ARLib {
template <typename T, typename U = typename InvUnwrap<T>::type>
constexpr U&& invfwd(typename RemoveReference<T>::type& t) noexcept {
    return static_cast<T&&>(t);
}
template <typename Res, typename Fn, typename... Args>
constexpr Res invoke_impl(detail::InvokeOther, Fn&& f, Args&&... args) {
    return Forward<Fn>(f)(Forward<Args>(args)...);
}
template <typename Res, typename MemFun, typename T, typename... Args>
constexpr Res invoke_impl(detail::InvokeMemfunRef, MemFun&& f, T&& t, Args&&... args) {
    return (invfwd<T>(t).*f)(Forward<Args>(args)...);
}
template <typename Res, typename MemFun, typename T, typename... Args>
constexpr Res invoke_impl(detail::InvokeMemfunDeref, MemFun&& f, T&& t, Args&&... args) {
    return ((*Forward<T>(t)).*f)(Forward<Args>(args)...);
}
template <typename Res, typename MemPtr, typename T>
constexpr Res invoke_impl(detail::InvokeMemobjRef, MemPtr&& f, T&& t) {
    return invfwd<T>(t).*f;
}
template <typename Res, typename MemPtr, typename T>
constexpr Res invoke_impl(detail::InvokeMemobjDeref, MemPtr&& f, T&& t) {
    return (*Forward<T>(t)).*f;
}
template <typename Callable, typename... Args>
constexpr typename InvokeResult<Callable, Args...>::type invoke(Callable&& fn, Args&&... args) noexcept {
    using result = detail::InvokeResultForFunc<Callable, Args...>;
    using type   = typename result::type;
    using tag    = typename result::invoke_type;
    return invoke_impl<type>(tag{}, Forward<Callable>(fn), Forward<Args>(args)...);
}
template <typename Res, typename Callable, typename... Args>
constexpr Res invoke_r(Callable&& fn, Args&&... args)
requires CallableWithRes<Callable, Res, Args...>
{
    using result = detail::InvokeResultForFunc<Callable, Args...>;
    using type   = typename result::type;
    using tag    = typename result::invoke_type;
    if constexpr (IsVoid<Res>::value)
        invoke_impl<type>(tag{}, Forward<Callable>(fn), Forward<Args>(args)...);
    else
        return invoke_impl<type>(tag{}, Forward<Callable>(fn), Forward<Args>(args)...);
}
}    // namespace ARLib
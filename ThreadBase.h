#pragma once
#ifndef DISABLE_THREADING
    #define THREADBASE_INCLUDED__

    #include "Functional.h"
    #include "Tuple.h"
    #include "TypeTraits.h"
    #include "UniquePtr.h"
    #include "XNative/thread/xnative_thread_merge.h"
namespace ARLib {
namespace detail {
    template <typename Tup, size_t... Indices>
    RetVal invoke_deferred(void* rawvals) noexcept {
        const UniquePtr<Tup> fnvals(static_cast<Tup*>(rawvals));
        const Tup& tup = *fnvals;
        invoke(move(get<Indices>(tup))...);
    #ifdef COMPILER_MSVC
        cond_do_broadcast_at_thread_exit();
    #endif
        return 0;
    }
    template <typename Tup, size_t... Indices>
    constexpr static auto get_deferred_invoke(IndexSequence<Indices...>) noexcept {
        return &invoke_deferred<Tup, Indices...>;
    }
}    // namespace detail
template <typename Fn, typename... Args>
ThreadT defer_execution(Fn&& functor, Args&&... args) {
    using FnTuple     = Tuple<DecayT<Fn>, DecayT<Args>...>;
    auto decay_copied = UniquePtr<FnTuple>{ EmplaceT<FnTuple>{}, Forward<Fn>(functor), Forward<Args>(args)... };
    constexpr auto invoker_proc = detail::get_deferred_invoke<FnTuple>(MakeIndexSequence<1 + sizeof...(Args)>{});
    auto [tr, success]          = ThreadNative::create(invoker_proc, decay_copied.get());
    if (success) { (void)decay_copied.release(); }
    return tr;
}
}    // namespace ARLib
#endif
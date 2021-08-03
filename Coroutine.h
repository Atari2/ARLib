#pragma once
#include "Compat.h"
#include "HashBase.h"
#include "Ordering.h"
#include "TypeTraits.h"

// this doesn't compile because... reasons?
// I can't use co_await & friends because:
// `error: coroutines require a handle class template; cannot find "std::coroutine_handle"`
// (that's according to gcc, other compilers give similar errors)
// Even thought my class isn't tied to the std:: at all (I even tried stubbing out and aliasing the std:: classes as can
// be seen below, but no dice, more info in the other comment)
// Probably something funky is going on in the compiler, e.g. it's hardcoded to expect std:: stuff.
// I really feel like this shouldn't be the case but I'm not a compiler writer so what do I know.

// uncommenting this define whenever I make this code work, for now, leaving it like this will make it so
// the compiler will error whenever you try to include this file
// #define WORKING_COROUTINES

#ifdef WORKING_COROUTINES
#ifdef WINDOWS
#define ALIGN(x) 0
#else
#define ALIGN(x) alignof(Promise)
#endif

namespace ARLib {
    template <class Result, class = void>
    struct CoroutineTraitsImpl {};

    template <class Result>
    struct CoroutineTraitsImpl<Result, VoidT<typename Result::promise_type>> {
        using promise_type = typename Result::promise_type;
    };

    template <class Result, class...>
    struct CoroutineTraits : CoroutineTraitsImpl<Result> {};

    template <class Promise = void>
    struct CoroutineHandle;

    template <>
    struct CoroutineHandle<void> {
        protected:
        void* m_fr_ptr;

        public:
        constexpr CoroutineHandle() noexcept : m_fr_ptr(0) {}
        constexpr CoroutineHandle(nullptr_t h) noexcept : m_fr_ptr(h) {}
        CoroutineHandle& operator=(nullptr_t) noexcept {
            m_fr_ptr = nullptr;
            return *this;
        }
        constexpr void* address() const noexcept { return m_fr_ptr; }
        constexpr static CoroutineHandle from_address(void* a) noexcept {
            CoroutineHandle self;
            self.m_fr_ptr = a;
            return self;
        }
        constexpr explicit operator bool() const noexcept { return bool(m_fr_ptr); }
        bool done() const noexcept { return __builtin_coro_done(m_fr_ptr); }
        void operator()() const { resume(); }
        void resume() const { __builtin_coro_resume(m_fr_ptr); }
        void destroy() const { __builtin_coro_destroy(m_fr_ptr); }
    };

    constexpr bool operator==(CoroutineHandle<> a, CoroutineHandle<> b) noexcept { return a.address() == b.address(); }
    Ordering operator<=>(CoroutineHandle<> a, CoroutineHandle<> b) noexcept {
        return CompareThreeWay(a.address(), b.address());
    }

    template <class Promise>
    struct CoroutineHandle {
        private:
        void* m_fr_ptr = nullptr;

        public:
        constexpr CoroutineHandle() noexcept = default;
        constexpr CoroutineHandle(nullptr_t) noexcept {}

        static CoroutineHandle from_promise(Promise& p) {
#ifdef WINDOWS
            const auto prom_ptr = const_cast<void*>(static_cast<const volatile void*>(addressof(p)));
            const auto frame_ptr = __builtin_coro_promise(prom_ptr, 0, true);
            CoroutineHandle result;
            result.m_fr_ptr = frame_ptr;
            return result;
#else
            CoroutineHandle self;
            self.m_fr_ptr = __builtin_coro_promise((char*)&p, alignof(Promise), true);
            return self;
#endif
        }

        CoroutineHandle& operator=(nullptr_t) noexcept {
            m_fr_ptr = nullptr;
            return *this;
        }

        constexpr void* address() const noexcept { return m_fr_ptr; }
        constexpr static CoroutineHandle from_address(void* a) noexcept {
            CoroutineHandle self;
            self.m_fr_ptr = a;
            return self;
        }
        constexpr operator CoroutineHandle<>() const noexcept { return CoroutineHandle<>::from_address(address()); }
        constexpr explicit operator bool() const noexcept { return bool(m_fr_ptr); }
        void operator()() const { resume(); }
        void resume() const { __builtin_coro_resume(m_fr_ptr); }
        void destroy() const { __builtin_coro_destroy(m_fr_ptr); }
        Promise& promise() const {
            void* t = __builtin_coro_promise(m_fr_ptr, ALIGN(x), false);
            return *static_cast<Promise*>(t);
        }
    };

    struct NoopCoroutinePromise {};

    template <>
    struct CoroutineHandle<NoopCoroutinePromise> {
        private:
        friend CoroutineHandle NoopCoroutine() noexcept;
        struct Frame {
            static void dummy_resume_destroy() {}
            void (*r)() = dummy_resume_destroy;
            void (*d)() = dummy_resume_destroy;
            struct NoopCoroutinePromise p;
        };
        static Frame m_s_fr;
        explicit CoroutineHandle() noexcept = default;
        void* m_fr_ptr = &m_s_fr;

        public:
        constexpr operator CoroutineHandle<>() const noexcept { return CoroutineHandle<>::from_address(address()); }
        constexpr explicit operator bool() const noexcept { return true; }
        constexpr bool done() const noexcept { return false; }
        void operator()() const noexcept {}
        void resume() const noexcept {}
        void destroy() const noexcept {}
        NoopCoroutinePromise& promise() const noexcept { return m_s_fr.p; }
        constexpr void* address() const noexcept { return m_fr_ptr; }
    };

    using NoopCoroutineHandle = CoroutineHandle<NoopCoroutinePromise>;
    inline NoopCoroutineHandle::Frame NoopCoroutineHandle::m_s_fr{};
    inline NoopCoroutineHandle NoopCoroutine() noexcept { return NoopCoroutineHandle(); }

    struct SuspendAlways {
        constexpr bool await_ready() const noexcept { return false; }
        constexpr void await_suspend(CoroutineHandle<>) const noexcept {}
        constexpr void await_resume() const noexcept {}
    };

    struct SuspendNever {
        constexpr bool await_ready() const noexcept { return true; }
        constexpr void await_suspend(CoroutineHandle<>) const noexcept {}
        constexpr void await_resume() const noexcept {}
    };

    template <class Promise>
    struct Hash<CoroutineHandle<Promise>> {
        [[nodiscard]] size_t operator()(const CoroutineHandle<Promise>& coro) const noexcept {
            return hash_representation(coro.address());
        }
    };
} // namespace ARLib

// the following is my naive attempt at tricking the compiler into thinking that std::coroutine_traits exists.
// Surprise: it doesn't work, as per comment at the top of file.
// This changes the error that gcc gives, it's now totally non-sensical (to be expected, as I'm messing with std:: and
// that's UB) and generates an ICE in MSVC's compiler. Sad times. Also apparently the version of clang that I'm using
// still has coroutines in std::experimental:: which is even more sad.

namespace std {
#ifdef COMPILER_CLANG
    namespace experimental {
#endif
        template <class Result, class...>
        struct coroutine_traits : ARLib::CoroutineTraitsImpl<Result> {};

        template <class Promise = void>
        using coroutine_handle = ARLib::CoroutineHandle<Promise>;

        using noop_coroutine_promise = ARLib::NoopCoroutinePromise;
        using noop_coroutine_handle = ARLib::NoopCoroutineHandle;

        using suspend_never = ARLib::SuspendNever;
        using suspend_always = ARLib::SuspendAlways;
#ifdef COMPILER_CLANG
    }
#endif
} // namespace std
#else
#error "Don't include coroutines, they don't work yet."
#endif
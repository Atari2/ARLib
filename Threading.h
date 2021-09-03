#pragma once
#ifndef DISABLE_THREADING
#include "HashBase.h"
#include "ThreadBase.h"

namespace ARLib {
    class Thread {
        ThreadT m_thread{};

        public:
        template <typename Tp>
        static constexpr inline bool NotSame = Not<IsSame<RemoveCvRefT<Tp>, Thread>>::value;
        static constexpr inline ThreadId NotAThread = ThreadId{};

        public:
        Thread() noexcept = default;
        template <typename Callable, typename... Args>
        requires NotSame<Callable>
        explicit Thread(Callable&& f, Args&&... args) { m_thread = defer_execution(f, args...); }
        Thread(const Thread&) = delete;
        Thread(Thread&& other) noexcept { swap(other); }
        Thread& operator=(const Thread&) = delete;
        Thread& operator=(Thread&& t) noexcept {
            if (joinable()) { arlib_terminate(); }
            swap(t);
            return *this;
        }
        bool joinable() { return !(ThreadNative::get_id(m_thread) == NotAThread); }
        ThreadId get_id() { return ThreadNative::get_id(m_thread); }
        ThreadT native_handle() { return m_thread; }
        void join() {
            if (!joinable()) { arlib_terminate(); }
            RetVal val = ThreadNative::retval_none();
            ThreadNative::join(m_thread, &val);
            m_thread = {};
        }
        void detach() {
            if (!joinable()) { arlib_terminate(); }
            ThreadNative::detach(m_thread);
            m_thread = {};
        }
        void swap(Thread& other) { ThreadNative::swap_id(m_thread, other.m_thread); }
        ~Thread() {
            if (joinable()) { arlib_terminate(); }
        }
    };
} // namespace ARLib
#endif
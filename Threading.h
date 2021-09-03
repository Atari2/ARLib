#pragma once
#ifndef DISABLE_THREADING
#include "HashBase.h"
#include "ThreadBase.h"

namespace ARLib {

    namespace detail {
        class MutexBase {
            protected:
            MutexT m_mutex = MutexNative::init_try_noret();
            constexpr MutexBase() noexcept = default;
            MutexBase(const MutexBase&) = delete;
            MutexBase& operator=(const MutexBase&) = delete;
        };
        class RecursiveMutexBase {
            protected:
            RecursiveMutexBase(const RecursiveMutexBase&) = delete;
            RecursiveMutexBase& operator=(const RecursiveMutexBase&) = delete;
            MutexT m_mutex = MutexNative::init_recursive_noret();
            RecursiveMutexBase() = default;
        };
    } // namespace detail

    class Mutex : private detail::MutexBase {
        public:
        using HandleType = MutexT*;
        constexpr Mutex() noexcept = default;
        ~Mutex() = default;

        Mutex(const Mutex&) = delete;
        Mutex& operator=(const Mutex&) = delete;

        void lock() {
            bool e = MutexNative::lock(m_mutex);
            if (!e) arlib_terminate();
        }

        bool try_lock() noexcept { return MutexNative::trylock(m_mutex); }
        void unlock() { MutexNative::unlock(m_mutex); }
        HandleType native_handle() noexcept { return &m_mutex; }
    };

    class RecursiveMutex : private detail::RecursiveMutexBase {
        public:
        using HandleType = MutexT*;
        RecursiveMutex() = default;
        ~RecursiveMutex() = default;
        RecursiveMutex(const RecursiveMutex&) = delete;
        RecursiveMutex& operator=(const RecursiveMutex&) = delete;

        void lock() {
            bool e = MutexNative::lock(m_mutex);
            if (!e) arlib_terminate();
        }
        bool try_lock() noexcept { return MutexNative::trylock(m_mutex); }
        void unlock() { MutexNative::unlock(m_mutex); }

        HandleType native_handle() noexcept { return &m_mutex; }
    };

    struct DeferLock {
        explicit DeferLock() = default;
    };
    struct TryToLock {
        explicit TryToLock() = default;
    };
    struct AdoptLock {
        explicit AdoptLock() = default;
    };
    inline constexpr DeferLock defer_lock{};
    inline constexpr TryToLock try_to_lock{};
    inline constexpr AdoptLock adopt_lock{};

    template <typename Mutex>
    class LockGuard {
        public:
        using MutexType = Mutex;
        explicit LockGuard(MutexType& m) : m_device(m) { m_device.lock(); }
        LockGuard(MutexType& m, AdoptLock) noexcept : m_device(m) {}
        ~LockGuard() { m_device.unlock(); }
        LockGuard(const LockGuard&) = delete;
        LockGuard& operator=(const LockGuard&) = delete;

        private:
        MutexType& m_device;
    };

    template <typename Mutex>
    class UniqueLock {
        Mutex* m_device;
        bool m_owns;

        public:
        using MutexType = Mutex;
        UniqueLock() noexcept : m_device(nullptr), m_owns(false) {}
        explicit UniqueLock(MutexType& m) : m_device(addressof(m)), m_owns(false) {
            lock();
            m_owns = true;
        }
        UniqueLock(MutexType& m, DeferLock) noexcept : m_device(addressof(m)), m_owns(false) {}

        UniqueLock(MutexType& m, TryToLock) : m_device(addressof(m)), m_owns(m_device->try_lock()) {}

        UniqueLock(MutexType& m, AdoptLock) noexcept : m_device(addressof(m)), m_owns(true) {}
        ~UniqueLock() {
            if (m_owns) unlock();
        }
        UniqueLock(const UniqueLock&) = delete;
        UniqueLock& operator=(const UniqueLock&) = delete;

        UniqueLock(UniqueLock&& u) noexcept : m_device(u.m_device), m_owns(u.m_owns) {
            u.m_device = 0;
            u.m_owns = false;
        }

        UniqueLock& operator=(UniqueLock&& u) noexcept {
            if (m_owns) unlock();

            UniqueLock(move(u)).swap(*this);

            u.m_device = 0;
            u.m_owns = false;

            return *this;
        }

        void lock() {
            if (!m_device)
                arlib_terminate();
            else if (m_owns)
                arlib_terminate();
            else {
                m_device->lock();
                m_owns = true;
            }
        }

        bool try_lock() {
            if (!m_device)
                arlib_terminate();
            else if (m_owns)
                arlib_terminate();
            else {
                m_owns = m_device->try_lock();
                return m_owns;
            }
        }

        void unlock() {
            if (!m_owns)
                arlib_terminate();
            else if (m_device) {
                m_device->unlock();
                m_owns = false;
            }
        }
        void swap(UniqueLock& u) noexcept {
            auto* ptr = m_device;
            m_device = u.m_device;
            u.m_device = ptr;
            bool owns = m_owns;
            m_owns = u.m_owns;
            u.m_owns = owns;
        }

        MutexType* release() noexcept {
            MutexType* ret = m_device;
            m_device = nullptr;
            m_owns = false;
            return ret;
        }

        bool owns_lock() const noexcept { return m_owns; }

        explicit operator bool() const noexcept { return owns_lock(); }

        MutexType* mutex() const noexcept { return m_device; }
    };

    template <typename Lock>
    inline UniqueLock<Lock> try_to_lock_(Lock& l) {
        return UniqueLock<Lock>{l, try_to_lock};
    }

    template <int Idx, bool Continue = true>
    struct try_to_lock_impl {
        template <typename... Lock>
        static void do_try_lock(Tuple<Lock&...>& locks, int& idx) {
            idx = Idx;
            auto lock = try_to_lock_(get<Idx>(locks));
            if (lock.owns_lock()) {
                constexpr bool cont = Idx + 2 < sizeof...(Lock);
                using try_locker = try_to_lock_impl<Idx + 1, cont>;
                try_locker::do_try_lock(locks, idx);
                if (idx == -1) lock.release();
            }
        }
    };

    template <int Idx>
    struct try_to_lock_impl<Idx, false> {
        template <typename... Lock>
        static void do_try_lock(Tuple<Lock&...>& locks, int& idx) {
            idx = Idx;
            auto lock = try_to_lock_(get<Idx>(locks));
            if (lock.owns_lock()) {
                idx = -1;
                lock.release();
            }
        }
    };

    template <typename L1, typename L2, typename... L3>
    void lock(L1& l1, L2& l2, L3&... l3) {
        while (true) {
            using try_locker = try_to_lock_impl<0, sizeof...(L3) != 0>;
            UniqueLock<L1> first(l1);
            int idx{};
            auto locks = tie(l2, l3...);
            try_locker::do_try_lock(locks, idx);
            if (idx == -1) {
                first.release();
                return;
            }
        }
    }

    template <typename... MutexTypes>
    class ScopedLock {
        public:
        explicit ScopedLock(MutexTypes&... m) : m_devices(tie(m...)) { lock(m...); }

        explicit ScopedLock(AdoptLock, MutexTypes&... m) noexcept :
            m_devices(tie(m...)) {} 

        ~ScopedLock() {
            apply([](auto&... m) { (m.unlock(), ...); }, m_devices);
        }

        ScopedLock(const ScopedLock&) = delete;
        ScopedLock& operator=(const ScopedLock&) = delete;

        private:
        Tuple<MutexTypes&...> m_devices;
    };

    template <>
    class ScopedLock<> {
        public:
        explicit ScopedLock() = default;
        explicit ScopedLock(AdoptLock) noexcept {}
        ~ScopedLock() = default;

        ScopedLock(const ScopedLock&) = delete;
        ScopedLock& operator=(const ScopedLock&) = delete;
    };

    template <typename Mutex>
    class ScopedLock<Mutex> {
        public:
        using MutexType = Mutex;

        explicit ScopedLock(MutexType& m) : m_device(m) { m_device.lock(); }

        explicit ScopedLock(AdoptLock, MutexType& m) noexcept : m_device(m) {}

        ~ScopedLock() { m_device.unlock(); }

        ScopedLock(const ScopedLock&) = delete;
        ScopedLock& operator=(const ScopedLock&) = delete;

        private:
        MutexType& m_device;
    };

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
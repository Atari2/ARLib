#pragma once
#ifndef DISABLE_THREADING
    #include "CharConv.hpp"
    #include "HashBase.hpp"
    #include "Ordering.hpp"
    #include "PrintInfo.hpp"
    #include "ThreadBase.hpp"
    #include "Atomic.hpp"
namespace ARLib {
namespace detail {
    class MutexBase {
        protected:
        MutexT m_mutex                         = MutexNative::init_try_noret();
        constexpr MutexBase() noexcept         = default;
        MutexBase(const MutexBase&)            = delete;
        MutexBase(MutexBase&&)                 = delete;
        MutexBase& operator=(const MutexBase&) = delete;
        MutexBase& operator=(MutexBase&&)      = delete;
    };
    class RecursiveMutexBase {
        protected:
        MutexT m_mutex                                           = MutexNative::init_recursive_noret();
        constexpr RecursiveMutexBase() noexcept                  = default;
        RecursiveMutexBase(const RecursiveMutexBase&)            = delete;
        RecursiveMutexBase(RecursiveMutexBase&&)                 = delete;
        RecursiveMutexBase& operator=(const RecursiveMutexBase&) = delete;
        RecursiveMutexBase& operator=(RecursiveMutexBase&&)      = delete;
    };
}    // namespace detail
class Mutex : private detail::MutexBase {
    public:
    using HandleType           = MutexT*;
    using ConstHandleType      = const MutexT*;
    constexpr Mutex() noexcept = default;
    ~Mutex()                   = default;

    Mutex(const Mutex&)            = delete;
    Mutex& operator=(const Mutex&) = delete;
    Mutex(Mutex&&)                 = delete;
    Mutex& operator=(Mutex&&)      = delete;
    bool operator==(const Mutex& other) const { return native_handle() == other.native_handle(); }
    bool operator!=(const Mutex& other) const { return native_handle() != other.native_handle(); }
    Ordering operator<=>(const Mutex& other) const { return CompareThreeWay(native_handle(), other.native_handle()); }
    void lock() {
        bool e = MutexNative::lock(m_mutex);
        if (!e) arlib_terminate();
    }
    bool try_lock() noexcept { return MutexNative::trylock(m_mutex); }
    void unlock() { MutexNative::unlock(m_mutex); }
    HandleType native_handle() noexcept { return &m_mutex; }
    ConstHandleType native_handle() const noexcept { return &m_mutex; }
};
class RecursiveMutex : private detail::RecursiveMutexBase {
    public:
    using HandleType                                 = MutexT*;
    using ConstHandleType                            = const MutexT*;
    RecursiveMutex()                                 = default;
    ~RecursiveMutex()                                = default;
    RecursiveMutex(const RecursiveMutex&)            = delete;
    RecursiveMutex& operator=(const RecursiveMutex&) = delete;
    RecursiveMutex(RecursiveMutex&&)                 = delete;
    RecursiveMutex& operator=(RecursiveMutex&&)      = delete;
    bool operator==(const RecursiveMutex& other) const { return native_handle() == other.native_handle(); }
    bool operator!=(const RecursiveMutex& other) const { return native_handle() != other.native_handle(); }
    Ordering operator<=>(const RecursiveMutex& other) const {
        return CompareThreeWay(native_handle(), other.native_handle());
    }
    void lock() {
        bool e = MutexNative::lock(m_mutex);
        if (!e) arlib_terminate();
    }
    bool try_lock() noexcept { return MutexNative::trylock(m_mutex); }
    void unlock() { MutexNative::unlock(m_mutex); }
    HandleType native_handle() noexcept { return &m_mutex; }
    ConstHandleType native_handle() const noexcept { return &m_mutex; }
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
constexpr inline DeferLock defer_lock{};
constexpr inline TryToLock try_to_lock{};
constexpr inline AdoptLock adopt_lock{};
template <typename Mutex>
class LockGuard {
    public:
    using MutexType      = Mutex;
    using ConstMutexType = const Mutex;
    explicit LockGuard(MutexType& m) : m_device(m) { m_device.lock(); }
    LockGuard(MutexType& m, AdoptLock) noexcept : m_device(m) {}
    ~LockGuard() { m_device.unlock(); }
    LockGuard(const LockGuard&)            = delete;
    LockGuard& operator=(const LockGuard&) = delete;
    MutexType& device() { return m_device; }
    ConstMutexType& device() const { return m_device; }

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
    UniqueLock(const UniqueLock&)            = delete;
    UniqueLock& operator=(const UniqueLock&) = delete;
    bool operator==(const UniqueLock& other) const { return other.mutex() == mutex(); }
    bool operator!=(const UniqueLock& other) const { return other.mutex() != mutex(); }
    UniqueLock(UniqueLock&& u) noexcept : m_device(u.m_device), m_owns(u.m_owns) {
        u.m_device = 0;
        u.m_owns   = false;
    }
    UniqueLock& operator=(UniqueLock&& u) noexcept {
        if (m_owns) unlock();

        UniqueLock(move(u)).swap(*this);

        u.m_device = 0;
        u.m_owns   = false;

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
        auto* ptr  = m_device;
        m_device   = u.m_device;
        u.m_device = ptr;
        bool owns  = m_owns;
        m_owns     = u.m_owns;
        u.m_owns   = owns;
    }
    MutexType* release() noexcept {
        MutexType* ret = m_device;
        m_device       = nullptr;
        m_owns         = false;
        return ret;
    }
    bool owns_lock() const noexcept { return m_owns; }
    explicit operator bool() const noexcept { return owns_lock(); }
    MutexType* mutex() const noexcept { return m_device; }
};
template <typename Lock>
inline UniqueLock<Lock> try_to_lock_(Lock& l) {
    return UniqueLock<Lock>{ l, try_to_lock };
}
template <int Idx, bool Continue = true>
struct try_to_lock_impl {
    template <typename... Lock>
    static void do_try_lock(Tuple<Lock&...>& locks, int& idx) {
        idx       = Idx;
        auto lock = try_to_lock_(get<Idx>(locks));
        if (lock.owns_lock()) {
            constexpr bool cont = Idx + 2 < sizeof...(Lock);
            using try_locker    = try_to_lock_impl<Idx + 1, cont>;
            try_locker::do_try_lock(locks, idx);
            if (idx == -1) lock.release();
        }
    }
};
template <int Idx>
struct try_to_lock_impl<Idx, false> {
    template <typename... Lock>
    static void do_try_lock(Tuple<Lock&...>& locks, int& idx) {
        idx       = Idx;
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
    explicit ScopedLock(AdoptLock, MutexTypes&... m) noexcept : m_devices(tie(m...)) {}
    ~ScopedLock() {
        apply([](auto&... m) { (m.unlock(), ...); }, m_devices);
    }
    ScopedLock(const ScopedLock&)            = delete;
    ScopedLock& operator=(const ScopedLock&) = delete;
    const auto& devices() const { return m_devices; }
    auto& devices() { return m_devices; }

    private:
    Tuple<MutexTypes&...> m_devices;
};
template <>
class ScopedLock<> {
    public:
    explicit ScopedLock() = default;
    explicit ScopedLock(AdoptLock) noexcept {}
    ~ScopedLock() = default;

    ScopedLock(const ScopedLock&)            = delete;
    ScopedLock& operator=(const ScopedLock&) = delete;
};
template <typename Mutex>
class ScopedLock<Mutex> {
    public:
    using MutexType = Mutex;
    explicit ScopedLock(MutexType& m) : m_device(m) { m_device.lock(); }
    explicit ScopedLock(AdoptLock, MutexType& m) noexcept : m_device(m) {}
    ~ScopedLock() { m_device.unlock(); }
    ScopedLock(const ScopedLock&)            = delete;
    ScopedLock& operator=(const ScopedLock&) = delete;
    const auto& device() const { return m_device; }

    private:
    MutexType& m_device;
};
class Thread {
    ThreadT m_thread{};

    public:
    template <typename Tp>
    constexpr static inline bool NotSame        = Not<IsSame<RemoveCvRefT<Tp>, Thread>>::value;
    constexpr static inline ThreadId NotAThread = ThreadId{};

    public:
    Thread() noexcept = default;
    template <typename Callable, typename... Args>
    requires NotSame<Callable>
    explicit Thread(Callable&& f, Args&&... args) {
        m_thread = defer_execution(Forward<Callable>(f), Forward<Args>(args)...);
    }
    Thread(const Thread&) = delete;
    Thread(Thread&& other) noexcept { swap(other); }
    Thread& operator=(const Thread&) = delete;
    Thread& operator=(Thread&& t) noexcept {
        if (joinable()) { arlib_terminate(); }
        swap(t);
        return *this;
    }
    bool operator==(const Thread& other) const { return get_id() == other.get_id(); }
    bool operator!=(const Thread& other) const { return get_id() != other.get_id(); }
    Ordering operator<=>(const Thread& other) const { return CompareThreeWay(get_id(), other.get_id()); }
    bool joinable() const { return !(ThreadNative::get_id(m_thread) == NotAThread); }
    ThreadId get_id() const { return ThreadNative::get_id(m_thread); }
    ThreadT native_handle() const { return m_thread; }
    void join() {
        if (!joinable()) { arlib_terminate(); }
        RetVal val                  = ThreadNative::retval_none();
        [[maybe_unused]] auto state = ThreadNative::join(m_thread, &val);
        HARD_ASSERT_FMT(
        static_cast<int>(state) == 0, "Thread %lu didn't join successfully, error number was %d\n",
        ThreadNative::get_id(m_thread), static_cast<int>(state)
        );
        m_thread = {};
    }
    void detach() {
        if (!joinable()) { arlib_terminate(); }
        [[maybe_unused]] auto state = ThreadNative::detach(m_thread);
        HARD_ASSERT_FMT(
        static_cast<int>(state) == 0, "Thread %lu didn't detach successfully, error number was %d\n",
        ThreadNative::get_id(m_thread), static_cast<int>(state)
        );
        m_thread = {};
    }
    void swap(Thread& other) { ThreadNative::swap(m_thread, other.m_thread); }
    ~Thread() {
        if (joinable()) { arlib_terminate(); }
    }
};
template <typename T>
class LockedPointer {
    Atomic<uintptr_t> m_storage;
    public:
    constexpr static uintptr_t lock_mask                = 3;
    constexpr static uintptr_t not_locked               = 0;
    constexpr static uintptr_t locked_notify_not_needed = 1;
    constexpr static uintptr_t locked_notify_needed     = 2;
    constexpr static uintptr_t ptr_value_mask           = ~lock_mask;
    constexpr LockedPointer() noexcept : m_storage{} {}
    explicit LockedPointer(const T* ptr) noexcept : m_storage{ reinterpret_cast<uintptr_t>(ptr) } {}
    LockedPointer(const LockedPointer&)            = delete;
    LockedPointer& operator=(const LockedPointer&) = delete;
    T* lock_and_load() noexcept {
        uintptr_t rep = m_storage.load();
        while (true) {
            switch (rep & lock_mask) {
                case not_locked:
                    if (m_storage.compare_exchange_weak(rep, rep | locked_notify_not_needed)) {
                        return reinterpret_cast<T*>(rep);
                    }
                    pause_sync();
                    break;
                case locked_notify_not_needed:
                    if (!m_storage.compare_exchange_weak(rep, (rep & ptr_value_mask) | locked_notify_needed)) {
                        pause_sync();
                        break;
                    }
                    rep = (rep & ptr_value_mask) | locked_notify_needed;
                    [[fallthrough]];
                case locked_notify_needed:
                    m_storage.wait(rep);
                    rep = m_storage.load();
                    break;
                default:
                    ASSERT_NOT_REACHED("Invalid bit pattern in LockedPointer");
                    break;
            }
        }
    }
    void store_and_unlock(const T* value) noexcept {
        const auto rep = m_storage.exchange(reinterpret_cast<uintptr_t>(value));
        if ((rep & lock_mask) == locked_notify_needed) { m_storage.notify_all(); }
    }
    T* unsafe_load() const noexcept { return reinterpret_cast<T*>(m_storage.load()); }
};
struct StopState;
class StopToken;
class StopCallbackBase {
    #ifdef COMPILER_MSVC
        #define ARLIB_CDECL __cdecl
    #else
        #define ARLIB_CDECL
    #endif
    friend StopState;
    using Callback = void(ARLIB_CDECL*)(StopCallbackBase*) noexcept;
    template <bool Transfer>
    void do_attach(ConditionalT<Transfer, StopState*&, StopState* const> state) noexcept;

    public:
    explicit StopCallbackBase(const Callback fn) noexcept : m_fn{ fn } {}
    StopCallbackBase(const StopCallbackBase&)            = delete;
    StopCallbackBase& operator=(const StopCallbackBase&) = delete;

    protected:
    inline void attach(const StopToken& token) noexcept;
    inline void attach(StopToken&& token) noexcept;
    inline void detach() noexcept;

    StopState* m_parent      = nullptr;
    StopCallbackBase* m_next = nullptr;
    StopCallbackBase* m_prev = nullptr;
    Callback m_fn;
};
struct StopState {
    Atomic<uint32_t> m_stop_tokens  = 1;
    Atomic<uint32_t> m_stop_sources = 2;
    LockedPointer<StopCallbackBase> m_callbacks;
    Atomic<const StopCallbackBase*> m_current_callback = nullptr;
    ThreadId m_stopping_thread{};
    bool stop_requested() const noexcept { return (m_stop_sources.load() & uint32_t{ 1 }) != 0; }
    bool stop_possible() const noexcept { return m_stop_sources.load() != 0; }
    bool request_stop() noexcept {
        if ((m_stop_sources.fetch_or(uint32_t{ 1 }) & uint32_t{ 1 }) != 0) { return false; }
        m_stopping_thread = ThreadNative::id();
        while (true) {
            auto head = m_callbacks.lock_and_load();
            m_current_callback.store(head);
            m_current_callback.notify_all();
            if (head == nullptr) {
                m_callbacks.store_and_unlock(nullptr);
                return true;
            }
            const auto next = exchange(head->m_next, nullptr);
            if (next != nullptr) { next->m_prev = nullptr; }
            m_callbacks.store_and_unlock(next);
            head->m_fn(head);
        }
    }
};
class StopSource;
class StopToken {
    friend StopSource;
    friend StopCallbackBase;
    StopState* m_state;
    explicit StopToken(StopState* const state) : m_state{ state } {}
    public:
    StopToken() noexcept : m_state{} {}
    StopToken(const StopToken& other) noexcept : m_state{ other.m_state } {
        const auto local = m_state;
        if (local != nullptr) { local->m_stop_tokens.fetch_add(1); }
    }
    StopToken(StopToken&& other) noexcept : m_state{ exchange(other.m_state, nullptr) } {}
    StopToken& operator=(const StopToken& other) noexcept {
        StopToken{ other }.swap(*this);
        return *this;
    }
    StopToken& operator=(StopToken&& other) noexcept {
        StopToken{ move(other) }.swap(*this);
        return *this;
    };
    ~StopToken() {
        const auto local = m_state;
        if (local != nullptr) {
            if (local->m_stop_tokens.fetch_sub(1) == 1) { delete local; }
        }
    }
    void swap(StopToken& other) noexcept { ARLib::swap(m_state, other.m_state); }
    bool stop_requested() const noexcept {
        const auto local = m_state;
        return local != nullptr && local->stop_requested();
    }
    bool stop_possible() const noexcept {
        const auto local = m_state;
        return local != nullptr && local->stop_possible();
    }
    friend bool operator==(const StopToken& lhs, const StopToken& rhs) noexcept = default;
    friend void swap(StopToken& lhs, StopToken& rhs) noexcept { ARLib::swap(lhs.m_state, rhs.m_state); }
};
struct NoStopState {};
class StopSource {
    StopState* m_state;
    public:
    StopSource() : m_state{ new StopState } {}
    explicit StopSource(NoStopState) noexcept : m_state{} {}
    StopSource(const StopSource& other) noexcept : m_state{ other.m_state } {
        const auto local = m_state;
        if (local != nullptr) { local->m_stop_sources.fetch_add(2); }
    }
    StopSource(StopSource&& other) noexcept : m_state{ exchange(other.m_state, nullptr) } {}
    StopSource& operator=(const StopSource& other) noexcept {
        StopSource{ other }.swap(*this);
        return *this;
    }
    StopSource& operator=(StopSource&& other) noexcept {
        StopSource{ move(other) }.swap(*this);
        return *this;
    }
    ~StopSource() {
        const auto local = m_state;
        if (local != nullptr) {
            if ((local->m_stop_sources.fetch_sub(2) >> 1) == 1) {
                if (local->m_stop_tokens.fetch_sub(1) == 1) { delete local; }
            }
        }
    }
    void swap(StopSource& other) noexcept { ARLib::swap(m_state, other.m_state); }
    StopToken get_token() const noexcept {
        const auto local = m_state;
        if (local != nullptr) { local->m_stop_tokens.fetch_add(1); }
        return StopToken{ local };
    }
    bool stop_requested() const noexcept {
        const auto local = m_state;
        return local != nullptr && local->stop_requested();
    }
    bool stop_possible() const noexcept { return m_state != nullptr; }
    bool request_stop() noexcept {
        const auto local = m_state;
        return local && local->request_stop();
    }
    friend bool operator==(const StopSource& lhs, const StopSource& rhs) noexcept = default;
    friend void swap(StopSource& lhs, StopSource& rhs) noexcept { ARLib::swap(lhs.m_state, rhs.m_state); }
};
template <bool Transfer>
void StopCallbackBase::do_attach(ConditionalT<Transfer, StopState*&, StopState* const> raw) noexcept {
    const auto state = raw;
    if (state == nullptr) { return; }

    auto local_sources = state->m_stop_sources.load();
    if ((local_sources & uint32_t{ 1 }) != 0) {
        m_fn(this);
        return;
    }

    if (local_sources == 0) { return; }

    auto head     = state->m_callbacks.lock_and_load();
    local_sources = state->m_stop_sources.load();
    if ((local_sources & uint32_t{ 1 }) != 0) {
        state->m_callbacks.store_and_unlock(head);
        m_fn(this);
        return;
    }

    if (state != 0) {
        m_parent = state;
        m_next   = head;
        if constexpr (Transfer) {
            raw = nullptr;
        } else {
            state->m_stop_tokens.fetch_add(1);
        }

        if (head != nullptr) { head->m_prev = this; }

        head = this;
    }

    state->m_callbacks.store_and_unlock(head);
}
inline void StopCallbackBase::attach(const StopToken& token) noexcept {
    this->do_attach<false>(token.m_state);
}
inline void StopCallbackBase::attach(StopToken&& token) noexcept {
    this->do_attach<true>(token.m_state);
}
inline void StopCallbackBase::detach() noexcept {
    StopToken token{ m_parent };
    if (token.m_state == nullptr) { return; }

    auto head = token.m_state->m_callbacks.lock_and_load();
    if (this == head) {
        const auto local_next = m_next;
        if (local_next != nullptr) { local_next->m_prev = nullptr; }

        token.m_state->m_callbacks.store_and_unlock(m_next);
        return;
    }

    const auto local_prev = m_prev;
    if (local_prev != nullptr) {
        const auto local_next = m_next;
        if (local_next != nullptr) { m_next->m_prev = local_prev; }

        m_prev->m_next = local_next;
        token.m_state->m_callbacks.store_and_unlock(head);
        return;
    }
    if (token.m_state->m_current_callback.load() != this || token.m_state->m_stopping_thread == ThreadNative::id()) {
        token.m_state->m_callbacks.store_and_unlock(head);
        return;
    }
    token.m_state->m_callbacks.store_and_unlock(head);
    token.m_state->m_current_callback.wait(this);
}
class JThread {
    Thread m_thread;
    StopSource m_source;
    void try_cancel_and_join() noexcept {
        if (m_thread.joinable()) {
            m_source.request_stop();
            m_thread.join();
        }
    }
    public:
    JThread() noexcept : m_thread{}, m_source{ NoStopState{} } {}
    template <class Fn, class... Args>
    requires(!SameAs<RemoveCvRefT<Fn>, JThread>)
    explicit JThread(Fn&& fx, Args&&... args) {
        if constexpr (CallableWith<DecayT<Fn>, StopToken, DecayT<Args>...>) {
            m_thread = Thread{ Forward<Fn>(fx), m_source.get_token(), Forward<Args>(args)... };
        } else {
            m_thread = Thread{ Forward<Fn>(fx), Forward<Args>(args)... };
        }
    }
    ~JThread() { try_cancel_and_join(); }
    JThread(const JThread&)           = delete;
    JThread(JThread&&) noexcept       = default;
    JThread operator=(const JThread&) = delete;
    JThread& operator=(JThread&& other) noexcept {
        if (this == addressof(other)) { return *this; }
        try_cancel_and_join();
        m_thread = move(other.m_thread);
        m_source = move(other.m_source);
        return *this;
    }
    void swap(JThread& other) noexcept {
        m_thread.swap(other.m_thread);
        m_source.swap(other.m_source);
    }
    bool joinable() const noexcept { return m_thread.joinable(); }
    void join() { m_thread.join(); }
    void detach() { m_thread.detach(); }
    auto get_id() const noexcept { return m_thread.get_id(); }
    auto native_handle() noexcept { return m_thread.native_handle(); }
    StopSource get_stop_source() noexcept { return m_source; }
    StopToken get_stop_token() noexcept { return m_source.get_token(); }
    bool request_stop() noexcept { return m_source.request_stop(); }
    friend void swap(JThread& lhs, JThread& rhs) noexcept { lhs.swap(rhs); }
};
class ThisThread {
    public:
    static auto id() { return ThreadNative::id(); }
    static auto sleep(int64_t microseconds) { return ThreadNative::sleep(microseconds); }
};
template <>
struct Hash<Mutex> {
    [[nodiscard]] size_t operator()(const Mutex& key) const noexcept {
        return hash_representation(key.native_handle());
    }
};
template <>
struct Hash<RecursiveMutex> {
    [[nodiscard]] size_t operator()(const RecursiveMutex& key) const noexcept {
        return hash_representation(key.native_handle());
    }
};
template <typename T>
struct Hash<UniqueLock<T>> {
    [[nodiscard]] size_t operator()(const UniqueLock<T>& key) const noexcept {
        return hash_representation(key.mutex()->native_handle());
    }
};
template <>
struct Hash<Thread> {
    [[nodiscard]] size_t operator()(const Thread& key) const noexcept { return key.get_id(); }
};
template <>
struct PrintInfo<Mutex> {
    const Mutex& m_mutex;
    using PtrT = decltype(m_mutex.native_handle());
    explicit PrintInfo(const Mutex& mutex) : m_mutex(mutex) {}
    String repr() const { return "Mutex { "_s + PrintInfo<PtrT>{ m_mutex.native_handle() }.repr() + " }"_s; }
};
template <>
struct PrintInfo<RecursiveMutex> {
    const RecursiveMutex& m_mutex;
    using PtrT = decltype(m_mutex.native_handle());
    explicit PrintInfo(const RecursiveMutex& mutex) : m_mutex(mutex) {}
    String repr() const { return "RecursiveMutex { "_s + PrintInfo<PtrT>{ m_mutex.native_handle() }.repr() + " }"_s; }
};
template <Printable M>
struct PrintInfo<LockGuard<M>> {
    const LockGuard<M>& m_lock;
    explicit PrintInfo(const LockGuard<M>& lock) : m_lock(lock) {}
    String repr() const { return "LockGuard { "_s + PrintInfo<M>{ m_lock.device() }.repr() + " }"_s; }
};
template <Printable M>
struct PrintInfo<UniqueLock<M>> {
    const UniqueLock<M>& m_lock;
    explicit PrintInfo(const UniqueLock<M>& lock) : m_lock(lock) {}
    String repr() const { return "UniqueLock { "_s + PrintInfo<M>{ *m_lock.mutex() }.repr() + " }"_s; }
};
template <Printable... Args>
struct PrintInfo<ScopedLock<Args...>> {
    const ScopedLock<Args...>& m_lock;
    explicit PrintInfo(const ScopedLock<Args...>& lock) : m_lock(lock) {}
    String repr() const { return "ScopedLock { "_s + PrintInfo<Tuple<Args&...>>{ m_lock.devices() }.repr() + " }"_s; }
};
template <>
struct PrintInfo<ScopedLock<Mutex>> {
    const ScopedLock<Mutex>& m_lock;
    explicit PrintInfo(const ScopedLock<Mutex>& lock) : m_lock(lock) {}
    String repr() const { return "ScopedLock { "_s + PrintInfo<Mutex>{ m_lock.device() }.repr() + " }"_s; }
};
template <>
struct PrintInfo<ScopedLock<>> {
    const ScopedLock<>& m_lock;
    explicit PrintInfo(const ScopedLock<>& lock) : m_lock(lock) {}
    String repr() const { return "ScopedLock { empty scoped lock }"_s; }
};
template <>
struct PrintInfo<Thread> {
    const Thread& m_thread;
    explicit PrintInfo(const Thread& thread) : m_thread(thread) {}
    String repr() const { return "Thread { "_s + IntToStr(m_thread.get_id()) + " }"_s; }
};
}    // namespace ARLib
#endif

#define INCLUDED_FROM_OWN_CPP___
#include "xnative_thread_unix.h"

#ifdef UNIX_OR_MINGW
#include <pthread.h>
namespace ARLib {

    template <typename T, typename U>
    T _to_(U in) {
        return reinterpret_cast<T>(in);
    }

    int pthread_attr_destroy(PthreadAttr* attr) { return ::pthread_attr_destroy(_to_<pthread_attr_t*>(attr)); }
    int pthread_attr_getdetachstate(const PthreadAttr* attr, int* state) {
        return ::pthread_attr_getdetachstate(_to_<const pthread_attr_t*>(attr), state);
    }
#ifndef ON_MINGW
    int pthread_attr_getguardsize(const PthreadAttr* attr, size_t* gsize) {
        return ::pthread_attr_getguardsize(_to_<const pthread_attr_t*>(attr), gsize);
    }
#endif
    int pthread_attr_getinheritsched(const PthreadAttr* attr, int* sched) {
        return ::pthread_attr_getinheritsched(_to_<const pthread_attr_t*>(attr), sched);
    }
    int pthread_attr_getschedparam(const PthreadAttr* attr, SchedParam* schedparam) {
        return ::pthread_attr_getschedparam(_to_<const pthread_attr_t*>(attr), _to_<sched_param*>(schedparam));
    }
    int pthread_attr_getschedpolicy(const PthreadAttr* attr, int* policy) {
        return ::pthread_attr_getschedpolicy(_to_<const pthread_attr_t*>(attr), policy);
    }
    int pthread_attr_getscope(const PthreadAttr* attr, int* scope) {
        return ::pthread_attr_getscope(_to_<const pthread_attr_t*>(attr), scope);
    }
    int pthread_attr_init(PthreadAttr* attr) { return ::pthread_attr_init(_to_<pthread_attr_t*>(attr)); }
    int pthread_attr_setdetachstate(PthreadAttr* attr, int state) {
        return ::pthread_attr_setdetachstate(_to_<pthread_attr_t*>(attr), state);
    }
#ifndef ON_MINGW
    int pthread_attr_setguardsize(PthreadAttr* attr, size_t size) {
        return ::pthread_attr_setguardsize(_to_<pthread_attr_t*>(attr), size);
    }
#endif
    int pthread_attr_setinheritsched(PthreadAttr* attr, int sched) {
        return ::pthread_attr_setinheritsched(_to_<pthread_attr_t*>(attr), sched);
    }
    int pthread_attr_setschedparam(PthreadAttr* attr, const SchedParam* schedparam) {
        return ::pthread_attr_setschedparam(_to_<pthread_attr_t*>(attr), _to_<const sched_param*>(schedparam));
    }
    int pthread_attr_setschedpolicy(PthreadAttr* attr, int schedpolicy) {
        return ::pthread_attr_setschedpolicy(_to_<pthread_attr_t*>(attr), schedpolicy);
    }
    int pthread_attr_setscope(PthreadAttr* attr, int scope) {
        return ::pthread_attr_setscope(_to_<pthread_attr_t*>(attr), scope);
    }
    int pthread_cancel(Pthread thread) { return ::pthread_cancel(thread); }
    int pthread_cond_broadcast(PthreadCond* cond) { return ::pthread_cond_broadcast(_to_<pthread_cond_t*>(cond)); }
    int pthread_cond_destroy(PthreadCond* cond) { return ::pthread_cond_destroy(_to_<pthread_cond_t*>(cond)); }
    int pthread_cond_init(PthreadCond* cond, const PthreadCondAttr* condattr) {
        return ::pthread_cond_init(_to_<pthread_cond_t*>(cond), _to_<const pthread_condattr_t*>(condattr));
    }
    int pthread_cond_signal(PthreadCond* cond) { return ::pthread_cond_signal(_to_<pthread_cond_t*>(cond)); }
    int pthread_cond_timedwait(PthreadCond* cond, PthreadMutex* mutex, const TimeSpec* spec) {
        return ::pthread_cond_timedwait(_to_<pthread_cond_t*>(cond), _to_<pthread_mutex_t*>(mutex),
                                        _to_<const timespec*>(spec));
    }
    int pthread_cond_wait(PthreadCond* cond, PthreadMutex* mutex) {
        return ::pthread_cond_wait(_to_<pthread_cond_t*>(cond), _to_<pthread_mutex_t*>(mutex));
    }
    int pthread_condattr_destroy(PthreadCondAttr* attr) {
        return ::pthread_condattr_destroy(_to_<pthread_condattr_t*>(attr));
    }
    int pthread_condattr_getpshared(const PthreadCondAttr* attr, int* pshared) {
        return ::pthread_condattr_getpshared(_to_<const pthread_condattr_t*>(attr), pshared);
    }
    int pthread_condattr_init(PthreadCondAttr* attr) {
        return ::pthread_condattr_init(_to_<pthread_condattr_t*>(attr));
    }
    int pthread_condattr_setpshared(PthreadCondAttr* attr, int pshared) {
        return ::pthread_condattr_setpshared(_to_<pthread_condattr_t*>(attr), pshared);
    }
    int pthread_create(Pthread* pthread, const PthreadAttr* attr, void* (*routine)(void*), void* arg) {
        return ::pthread_create(_to_<pthread_t*>(pthread), _to_<const pthread_attr_t*>(attr), routine, arg);
    }
    int pthread_detach(Pthread thread) { return ::pthread_detach(thread); }
    int pthread_equal(Pthread thread1, Pthread thread2) { return ::pthread_equal(thread1, thread2); }
    void pthread_exit(void* value_ptr) { return ::pthread_exit(value_ptr); }
    int pthread_getconcurrency(void) { return ::pthread_getconcurrency(); }
    int pthread_getschedparam(Pthread thread, int* policy, SchedParam* param) {
        return ::pthread_getschedparam(thread, policy, _to_<sched_param*>(param));
    }
    void* pthread_getspecific(PthreadKey key) { return ::pthread_getspecific(key); }
    int pthread_join(Pthread thread, void** value_ptr) { return ::pthread_join(thread, value_ptr); }
    int pthread_key_create(PthreadKey* key, void (*routine)(void*)) { return ::pthread_key_create(key, routine); }
    int pthread_key_delete(PthreadKey key) { return ::pthread_key_delete(key); }
    int pthread_mutex_destroy(PthreadMutex* mutex) { return ::pthread_mutex_destroy(_to_<pthread_mutex_t*>(mutex)); }
#ifndef ON_MINGW
    int pthread_mutex_getprioceiling(const PthreadMutex* mutex, int* prio) {
        return ::pthread_mutex_getprioceiling(_to_<const pthread_mutex_t*>(mutex), prio);
    }
#endif
    int pthread_mutex_init(PthreadMutex* mutex, const PthreadMutexAttr* attr) {
        return ::pthread_mutex_init(_to_<pthread_mutex_t*>(mutex), _to_<const pthread_mutexattr_t*>(attr));
    }
    int pthread_mutex_lock(PthreadMutex* mutex) { return ::pthread_mutex_lock(_to_<pthread_mutex_t*>(mutex)); }
#ifndef ON_MINGW
    int pthread_mutex_setprioceiling(PthreadMutex* mutex, int prio, int* prop_ptr) {
        return ::pthread_mutex_setprioceiling(_to_<pthread_mutex_t*>(mutex), prio, prop_ptr);
    }
#endif
    int pthread_mutex_trylock(PthreadMutex* mutex) { return ::pthread_mutex_trylock(_to_<pthread_mutex_t*>(mutex)); }
    int pthread_mutex_timedlock(PthreadMutex* mutex, const TimeSpec* spec) {
        return ::pthread_mutex_timedlock(_to_<pthread_mutex_t*>(mutex), _to_<const timespec*>(spec));
    }
    int pthread_mutex_unlock(PthreadMutex* mutex) { return ::pthread_mutex_unlock(_to_<pthread_mutex_t*>(mutex)); }
    int pthread_mutexattr_destroy(PthreadMutexAttr* attr) {
        return ::pthread_mutexattr_destroy(_to_<pthread_mutexattr_t*>(attr));
    }
    int pthread_mutexattr_getprioceiling(const PthreadMutexAttr* attr, int* prio) {
        return ::pthread_mutexattr_getprioceiling(_to_<const pthread_mutexattr_t*>(attr), prio);
    }
    int pthread_mutexattr_getprotocol(const PthreadMutexAttr* attr, int* protocol) {
        return ::pthread_mutexattr_getprotocol(_to_<const pthread_mutexattr_t*>(attr), protocol);
    }
    int pthread_mutexattr_getpshared(const PthreadMutexAttr* attr, int* pshared) {
        return ::pthread_mutexattr_getpshared(_to_<const pthread_mutexattr_t*>(attr), pshared);
    }
    int pthread_mutexattr_gettype(const PthreadMutexAttr* attr, int* type) {
        return ::pthread_mutexattr_gettype(_to_<const pthread_mutexattr_t*>(attr), type);
    }
    int pthread_mutexattr_init(PthreadMutexAttr* attr) {
        return ::pthread_mutexattr_init(_to_<pthread_mutexattr_t*>(attr));
    }
    int pthread_mutexattr_setprioceiling(PthreadMutexAttr* attr, int prio) {
        return ::pthread_mutexattr_setprioceiling(_to_<pthread_mutexattr_t*>(attr), prio);
    }
    int pthread_mutexattr_setprotocol(PthreadMutexAttr* attr, int protocol) {
        return ::pthread_mutexattr_setprotocol(_to_<pthread_mutexattr_t*>(attr), protocol);
    }
    int pthread_mutexattr_setpshared(PthreadMutexAttr* attr, int pshared) {
        return ::pthread_mutexattr_setpshared(_to_<pthread_mutexattr_t*>(attr), pshared);
    }
    int pthread_mutexattr_settype(PthreadMutexAttr* attr, int type) {
        return ::pthread_mutexattr_settype(_to_<pthread_mutexattr_t*>(attr), type);
    }
    int pthread_once(PthreadOnce* threadonce, void (*routine)(void)) {
        return ::pthread_once(_to_<pthread_once_t*>(threadonce), routine);
    }
    int pthread_rwlock_destroy(PthreadRWLock* lock) { return ::pthread_rwlock_destroy(_to_<pthread_rwlock_t*>(lock)); }
    int pthread_rwlock_init(PthreadRWLock* lock, const PthreadRWLockAttr* lockattr) {
        return ::pthread_rwlock_init(_to_<pthread_rwlock_t*>(lock), _to_<const pthread_rwlockattr_t*>(lockattr));
    }
    int pthread_rwlock_rdlock(PthreadRWLock* lock) { return ::pthread_rwlock_rdlock(_to_<pthread_rwlock_t*>(lock)); }
    int pthread_rwlock_tryrdlock(PthreadRWLock* lock) {
        return ::pthread_rwlock_tryrdlock(_to_<pthread_rwlock_t*>(lock));
    }
    int pthread_rwlock_trywrlock(PthreadRWLock* lock) {
        return ::pthread_rwlock_trywrlock(_to_<pthread_rwlock_t*>(lock));
    }
    int pthread_rwlock_unlock(PthreadRWLock* lock) { return ::pthread_rwlock_unlock(_to_<pthread_rwlock_t*>(lock)); }
    int pthread_rwlock_wrlock(PthreadRWLock* lock) { return ::pthread_rwlock_wrlock(_to_<pthread_rwlock_t*>(lock)); }
    int pthread_rwlockattr_destroy(PthreadRWLockAttr* lockattr) {
        return ::pthread_rwlockattr_destroy(_to_<pthread_rwlockattr_t*>(lockattr));
    }
    int pthread_rwlockattr_getpshared(LOCKATTR_CONST PthreadRWLockAttr* lockattr, int* pshared) {
        return ::pthread_rwlockattr_getpshared(_to_<LOCKATTR_CONST pthread_rwlockattr_t*>(lockattr), pshared);
    }
    int pthread_rwlockattr_init(PthreadRWLockAttr* lockattr) {
        return ::pthread_rwlockattr_init(_to_<pthread_rwlockattr_t*>(lockattr));
    }
    int pthread_rwlockattr_setpshared(PthreadRWLockAttr* lockattr, int pshared) {
        return ::pthread_rwlockattr_setpshared(_to_<pthread_rwlockattr_t*>(lockattr), pshared);
    }
    Pthread pthread_self(void) { return _to_<Pthread>(::pthread_self()); }
    int pthread_setcancelstate(int state, int* oldstate) { return ::pthread_setcancelstate(state, oldstate); }
    int pthread_setcanceltype(int type, int* oldtype) { return ::pthread_setcanceltype(type, oldtype); }
    int pthread_setconcurrency(int concurrency) { return ::pthread_setconcurrency(concurrency); }
    int pthread_setschedparam(Pthread thread, int policy, const SchedParam* schedparam) {
        return ::pthread_setschedparam(thread, policy, _to_<const sched_param*>(schedparam));
    }
    int pthread_setspecific(PthreadKey key, const void* specific) { return ::pthread_setspecific(key, specific); }
    void pthread_testcancel(void) { return ::pthread_testcancel(); }
} // namespace ARLib
#endif
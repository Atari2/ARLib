#ifndef INCLUDED_FROM_OWN_CPP___
#define INCLUDED_FROM_OWN_CPP___
#endif
#include "xnative_thread_unix.h"
#include "../../Conversion.h"

#ifdef UNIX_OR_MINGW
    #include <pthread.h>
namespace ARLib {
int pthread_attr_destroy(PthreadAttr* attr) {
    return ::pthread_attr_destroy(cast<pthread_attr_t*>(attr));
}
int pthread_attr_getdetachstate(const PthreadAttr* attr, int* state) {
    return ::pthread_attr_getdetachstate(cast<const pthread_attr_t*>(attr), state);
}
    #ifndef ON_MINGW
int pthread_attr_getguardsize(const PthreadAttr* attr, size_t* gsize) {
    return ::pthread_attr_getguardsize(cast<const pthread_attr_t*>(attr), gsize);
}
    #endif
int pthread_attr_getinheritsched(const PthreadAttr* attr, int* sched) {
    return ::pthread_attr_getinheritsched(cast<const pthread_attr_t*>(attr), sched);
}
int pthread_attr_getschedparam(const PthreadAttr* attr, SchedParam* schedparam) {
    return ::pthread_attr_getschedparam(cast<const pthread_attr_t*>(attr), cast<sched_param*>(schedparam));
}
int pthread_attr_getschedpolicy(const PthreadAttr* attr, int* policy) {
    return ::pthread_attr_getschedpolicy(cast<const pthread_attr_t*>(attr), policy);
}
int pthread_attr_getscope(const PthreadAttr* attr, int* scope) {
    return ::pthread_attr_getscope(cast<const pthread_attr_t*>(attr), scope);
}
int pthread_attr_init(PthreadAttr* attr) {
    return ::pthread_attr_init(cast<pthread_attr_t*>(attr));
}
int pthread_attr_setdetachstate(PthreadAttr* attr, int state) {
    return ::pthread_attr_setdetachstate(cast<pthread_attr_t*>(attr), state);
}
    #ifndef ON_MINGW
int pthread_attr_setguardsize(PthreadAttr* attr, size_t size) {
    return ::pthread_attr_setguardsize(cast<pthread_attr_t*>(attr), size);
}
    #endif
int pthread_attr_setinheritsched(PthreadAttr* attr, int sched) {
    return ::pthread_attr_setinheritsched(cast<pthread_attr_t*>(attr), sched);
}
int pthread_attr_setschedparam(PthreadAttr* attr, const SchedParam* schedparam) {
    return ::pthread_attr_setschedparam(cast<pthread_attr_t*>(attr), cast<const sched_param*>(schedparam));
}
int pthread_attr_setschedpolicy(PthreadAttr* attr, int schedpolicy) {
    return ::pthread_attr_setschedpolicy(cast<pthread_attr_t*>(attr), schedpolicy);
}
int pthread_attr_setscope(PthreadAttr* attr, int scope) {
    return ::pthread_attr_setscope(cast<pthread_attr_t*>(attr), scope);
}
int pthread_cancel(Pthread thread) {
    return ::pthread_cancel(thread);
}
int pthread_cond_broadcast(PthreadCond* cond) {
    return ::pthread_cond_broadcast(cast<pthread_cond_t*>(cond));
}
int pthread_cond_destroy(PthreadCond* cond) {
    return ::pthread_cond_destroy(cast<pthread_cond_t*>(cond));
}
int pthread_cond_init(PthreadCond* cond, const PthreadCondAttr* condattr) {
    return ::pthread_cond_init(cast<pthread_cond_t*>(cond), cast<const pthread_condattr_t*>(condattr));
}
int pthread_cond_signal(PthreadCond* cond) {
    return ::pthread_cond_signal(cast<pthread_cond_t*>(cond));
}
int pthread_cond_timedwait(PthreadCond* cond, PthreadMutex* mutex, const TimeSpec* spec) {
    return ::pthread_cond_timedwait(
    cast<pthread_cond_t*>(cond), cast<pthread_mutex_t*>(mutex), cast<const timespec*>(spec)
    );
}
int pthread_cond_wait(PthreadCond* cond, PthreadMutex* mutex) {
    return ::pthread_cond_wait(cast<pthread_cond_t*>(cond), cast<pthread_mutex_t*>(mutex));
}
int pthread_condattr_destroy(PthreadCondAttr* attr) {
    return ::pthread_condattr_destroy(cast<pthread_condattr_t*>(attr));
}
int pthread_condattr_getpshared(const PthreadCondAttr* attr, int* pshared) {
    return ::pthread_condattr_getpshared(cast<const pthread_condattr_t*>(attr), pshared);
}
int pthread_condattr_init(PthreadCondAttr* attr) {
    return ::pthread_condattr_init(cast<pthread_condattr_t*>(attr));
}
int pthread_condattr_setpshared(PthreadCondAttr* attr, int pshared) {
    return ::pthread_condattr_setpshared(cast<pthread_condattr_t*>(attr), pshared);
}
int pthread_create(Pthread* pthread, const PthreadAttr* attr, void* (*routine)(void*), void* arg) {
    return ::pthread_create(cast<pthread_t*>(pthread), cast<const pthread_attr_t*>(attr), routine, arg);
}
int pthread_detach(Pthread thread) {
    return ::pthread_detach(thread);
}
int pthread_equal(Pthread thread1, Pthread thread2) {
    return ::pthread_equal(thread1, thread2);
}
void pthread_exit(void* value_ptr) {
    return ::pthread_exit(value_ptr);
}
int pthread_getconcurrency(void) {
    return ::pthread_getconcurrency();
}
int pthread_getschedparam(Pthread thread, int* policy, SchedParam* param) {
    return ::pthread_getschedparam(thread, policy, cast<sched_param*>(param));
}
void* pthread_getspecific(PthreadKey key) {
    return ::pthread_getspecific(key);
}
int pthread_join(Pthread thread, void** value_ptr) {
    return ::pthread_join(thread, value_ptr);
}
int pthread_key_create(PthreadKey* key, void (*routine)(void*)) {
    return ::pthread_key_create(key, routine);
}
int pthread_key_delete(PthreadKey key) {
    return ::pthread_key_delete(key);
}
int pthread_mutex_destroy(PthreadMutex* mutex) {
    return ::pthread_mutex_destroy(cast<pthread_mutex_t*>(mutex));
}
    #ifndef ON_MINGW
int pthread_mutex_getprioceiling(const PthreadMutex* mutex, int* prio) {
    return ::pthread_mutex_getprioceiling(cast<const pthread_mutex_t*>(mutex), prio);
}
    #endif
int pthread_mutex_init(PthreadMutex* mutex, const PthreadMutexAttr* attr) {
    return ::pthread_mutex_init(cast<pthread_mutex_t*>(mutex), cast<const pthread_mutexattr_t*>(attr));
}
int pthread_mutex_lock(PthreadMutex* mutex) {
    return ::pthread_mutex_lock(cast<pthread_mutex_t*>(mutex));
}
    #ifndef ON_MINGW
int pthread_mutex_setprioceiling(PthreadMutex* mutex, int prio, int* prop_ptr) {
    return ::pthread_mutex_setprioceiling(cast<pthread_mutex_t*>(mutex), prio, prop_ptr);
}
    #endif
int pthread_mutex_trylock(PthreadMutex* mutex) {
    return ::pthread_mutex_trylock(cast<pthread_mutex_t*>(mutex));
}
int pthread_mutex_timedlock(PthreadMutex* mutex, const TimeSpec* spec) {
    return ::pthread_mutex_timedlock(cast<pthread_mutex_t*>(mutex), cast<const timespec*>(spec));
}
int pthread_mutex_unlock(PthreadMutex* mutex) {
    return ::pthread_mutex_unlock(cast<pthread_mutex_t*>(mutex));
}
int pthread_mutexattr_destroy(PthreadMutexAttr* attr) {
    return ::pthread_mutexattr_destroy(cast<pthread_mutexattr_t*>(attr));
}
int pthread_mutexattr_getprioceiling(const PthreadMutexAttr* attr, int* prio) {
    return ::pthread_mutexattr_getprioceiling(cast<const pthread_mutexattr_t*>(attr), prio);
}
int pthread_mutexattr_getprotocol(const PthreadMutexAttr* attr, int* protocol) {
    return ::pthread_mutexattr_getprotocol(cast<const pthread_mutexattr_t*>(attr), protocol);
}
int pthread_mutexattr_getpshared(const PthreadMutexAttr* attr, int* pshared) {
    return ::pthread_mutexattr_getpshared(cast<const pthread_mutexattr_t*>(attr), pshared);
}
int pthread_mutexattr_gettype(const PthreadMutexAttr* attr, int* type) {
    return ::pthread_mutexattr_gettype(cast<const pthread_mutexattr_t*>(attr), type);
}
int pthread_mutexattr_init(PthreadMutexAttr* attr) {
    return ::pthread_mutexattr_init(cast<pthread_mutexattr_t*>(attr));
}
int pthread_mutexattr_setprioceiling(PthreadMutexAttr* attr, int prio) {
    return ::pthread_mutexattr_setprioceiling(cast<pthread_mutexattr_t*>(attr), prio);
}
int pthread_mutexattr_setprotocol(PthreadMutexAttr* attr, int protocol) {
    return ::pthread_mutexattr_setprotocol(cast<pthread_mutexattr_t*>(attr), protocol);
}
int pthread_mutexattr_setpshared(PthreadMutexAttr* attr, int pshared) {
    return ::pthread_mutexattr_setpshared(cast<pthread_mutexattr_t*>(attr), pshared);
}
int pthread_mutexattr_settype(PthreadMutexAttr* attr, int type) {
    return ::pthread_mutexattr_settype(cast<pthread_mutexattr_t*>(attr), type);
}
int pthread_once(PthreadOnce* threadonce, void (*routine)(void)) {
    return ::pthread_once(cast<pthread_once_t*>(threadonce), routine);
}
int pthread_rwlock_destroy(PthreadRWLock* lock) {
    return ::pthread_rwlock_destroy(cast<pthread_rwlock_t*>(lock));
}
int pthread_rwlock_init(PthreadRWLock* lock, const PthreadRWLockAttr* lockattr) {
    return ::pthread_rwlock_init(cast<pthread_rwlock_t*>(lock), cast<const pthread_rwlockattr_t*>(lockattr));
}
int pthread_rwlock_rdlock(PthreadRWLock* lock) {
    return ::pthread_rwlock_rdlock(cast<pthread_rwlock_t*>(lock));
}
int pthread_rwlock_tryrdlock(PthreadRWLock* lock) {
    return ::pthread_rwlock_tryrdlock(cast<pthread_rwlock_t*>(lock));
}
int pthread_rwlock_trywrlock(PthreadRWLock* lock) {
    return ::pthread_rwlock_trywrlock(cast<pthread_rwlock_t*>(lock));
}
int pthread_rwlock_unlock(PthreadRWLock* lock) {
    return ::pthread_rwlock_unlock(cast<pthread_rwlock_t*>(lock));
}
int pthread_rwlock_wrlock(PthreadRWLock* lock) {
    return ::pthread_rwlock_wrlock(cast<pthread_rwlock_t*>(lock));
}
int pthread_rwlockattr_destroy(PthreadRWLockAttr* lockattr) {
    return ::pthread_rwlockattr_destroy(cast<pthread_rwlockattr_t*>(lockattr));
}
int pthread_rwlockattr_getpshared(LOCKATTR_CONST PthreadRWLockAttr* lockattr, int* pshared) {
    return ::pthread_rwlockattr_getpshared(cast<LOCKATTR_CONST pthread_rwlockattr_t*>(lockattr), pshared);
}
int pthread_rwlockattr_init(PthreadRWLockAttr* lockattr) {
    return ::pthread_rwlockattr_init(cast<pthread_rwlockattr_t*>(lockattr));
}
int pthread_rwlockattr_setpshared(PthreadRWLockAttr* lockattr, int pshared) {
    return ::pthread_rwlockattr_setpshared(cast<pthread_rwlockattr_t*>(lockattr), pshared);
}
Pthread pthread_self(void) {
    return cast<Pthread>(::pthread_self());
}
int pthread_setcancelstate(int state, int* oldstate) {
    return ::pthread_setcancelstate(state, oldstate);
}
int pthread_setcanceltype(int type, int* oldtype) {
    return ::pthread_setcanceltype(type, oldtype);
}
int pthread_setconcurrency(int concurrency) {
    return ::pthread_setconcurrency(concurrency);
}
int pthread_setschedparam(Pthread thread, int policy, const SchedParam* schedparam) {
    return ::pthread_setschedparam(thread, policy, cast<const sched_param*>(schedparam));
}
int pthread_setspecific(PthreadKey key, const void* specific) {
    return ::pthread_setspecific(key, specific);
}
void pthread_testcancel(void) {
    return ::pthread_testcancel();
}
}    // namespace ARLib
#endif

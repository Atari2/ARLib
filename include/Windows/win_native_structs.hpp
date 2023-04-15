#pragma once
#include "Compat.hpp"
#ifdef WINDOWS
    #include "Algorithm.hpp"
    #include "Conversion.hpp"
    #include "TypeTraits.hpp"
namespace ARLib {
using WinDword  = unsigned long;
using WinWord   = unsigned short;
using WinHandle = void*;
    #define ARLIB_INVALID_HANDLE_VALUE ((WinHandle)(__int64)-1)
    #define ARLIB_INFINITE_TIMEOUT     -1
namespace internal {
    struct SRWLock {
        void* Ptr;
    };
    using SRWLockPtr = SRWLock*;
    struct ConditionVariable {
        void* Ptr;
    };
    struct Overlapped {
        unsigned long long Internal;
        unsigned long long InternalHigh;
        union {
            struct {
                WinDword Offset;
                WinDword OffsetHigh;
            } DUMMYSTRUCTNAME;
            void* Pointer;
        } DUMMYUNIONNAME;
        WinHandle hEvent;
    };
    using LPOverlapped = Overlapped*;
    struct ProcessInformation {
        WinHandle hProcess;
        WinHandle hThread;
        WinDword dwProcessId;
        WinDword dwThreadId;
    };
    struct StartupInfoA {
        WinDword cb;
        char* lpReserved;
        char* lpDesktop;
        char* lpTitle;
        WinDword dwX;
        WinDword dwY;
        WinDword dwXSize;
        WinDword dwYSize;
        WinDword dwXCountChars;
        WinDword dwYCountChars;
        WinDword dwFillAttribute;
        WinDword dwFlags;
        WinWord wShowWindow;
        WinWord cbReserved2;
        unsigned char* lpReserved2;
        WinHandle hStdInput;
        WinHandle hStdOutput;
        WinHandle hStdError;
    };
    using LPOverlappedCompletionRoutine =
    void(__stdcall*)(WinDword dwErrorCode, WinDword dwNumberOfBytesTransfered, LPOverlapped lpOverlapped);
}    // namespace internal
class CriticalSection {
    public:
    CriticalSection();

    ~CriticalSection()                                 = delete;
    CriticalSection(const CriticalSection&)            = delete;
    CriticalSection& operator=(const CriticalSection&) = delete;

    void destroy();

    void lock();

    bool try_lock();

    bool try_lock_for(unsigned int);

    void unlock();

    internal::SRWLockPtr native_handle();

    static void Create(CriticalSection*);

    private:
    internal::SRWLock m_srw_lock;
};
class Win32ConditionVariable {
    public:
    Win32ConditionVariable();

    ~Win32ConditionVariable()                              = delete;
    Win32ConditionVariable(const Win32ConditionVariable&)  = delete;
    Win32ConditionVariable& operator=(const Win32ConditionVariable&) = delete;

    void destroy();

    void wait(CriticalSection* lock);

    bool wait_for(CriticalSection* lock, unsigned int timeout);

    void notify_one();

    void notify_all();

    static void Create(Win32ConditionVariable*);

    private:
    internal::ConditionVariable m_condition_variable;
};
using ThreadIdImplType = unsigned int;
struct ThreadImplType {
    void* _Hnd;
    ThreadIdImplType _Id;
};
constexpr size_t CriticalSectionSize        = 64;
constexpr size_t ConditionlVariableSize     = 72;
constexpr size_t CriticalSectionAlignment   = 8;
constexpr size_t ConditionVariableAlignment = 8;

constexpr size_t CriticalSectionMaxSize        = max_bt(CriticalSectionSize, sizeof(CriticalSection));
constexpr size_t ConditionVariableMaxSize      = max_bt(ConditionlVariableSize, sizeof(Win32ConditionVariable));
constexpr size_t CriticalSectionMaxAlignment   = max_bt(CriticalSectionAlignment, alignof(CriticalSection));
constexpr size_t ConditionVariableMaxAlignment = max_bt(ConditionVariableAlignment, alignof(Win32ConditionVariable));
struct MutexInternalImplType {
    int type;
    AlignedStorageT<CriticalSectionMaxSize, CriticalSectionMaxAlignment> cs;
    long thread_id;
    int count;
    CriticalSection* _get_cs() { return reinterpret_cast<CriticalSection*>(&cs); }
};
struct CondInternalImplType {
    AlignedStorageT<ConditionVariableMaxSize, ConditionVariableMaxAlignment> cv;
    Win32ConditionVariable* _get_cv() noexcept { return reinterpret_cast<Win32ConditionVariable*>(&cv); }
};
using mutex_internal_imp_t = MutexInternalImplType;
using cnd_internal_imp_t   = CondInternalImplType;

}    // namespace ARLib
#endif

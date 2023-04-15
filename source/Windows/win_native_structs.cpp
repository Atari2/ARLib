#include "Windows/win_native_structs.hpp"
#include "Assertion.hpp"

#include <Windows.h>
namespace ARLib {
#define VERIFY_WIN32_STRUCT(win32_struct, arlib_struct)                                                                \
    static_assert(                                                                                                     \
    sizeof(win32_struct) == sizeof(arlib_struct),                                                                      \
    "Size of struct " #arlib_struct " is wrong when compared to the original " #win32_struct                           \
    );                                                                                                                 \
    static_assert(                                                                                                     \
    alignof(win32_struct) == alignof(arlib_struct),                                                                    \
    "Alignment of struct " #arlib_struct " is wrong when compared to the original " #win32_struct                      \
    )

VERIFY_WIN32_STRUCT(SRWLOCK, internal::SRWLock);
VERIFY_WIN32_STRUCT(CONDITION_VARIABLE, internal::ConditionVariable);
VERIFY_WIN32_STRUCT(OVERLAPPED, internal::Overlapped);
VERIFY_WIN32_STRUCT(PROCESS_INFORMATION, internal::ProcessInformation);
VERIFY_WIN32_STRUCT(STARTUPINFOA, internal::StartupInfoA);
VERIFY_WIN32_STRUCT(WORD, WinWord);
VERIFY_WIN32_STRUCT(DWORD, WinDword);
VERIFY_WIN32_STRUCT(HANDLE, WinHandle);
VERIFY_WIN32_STRUCT(LPOVERLAPPED_COMPLETION_ROUTINE, internal::LPOverlappedCompletionRoutine);
static_assert(ARLIB_INFINITE_TIMEOUT == INFINITE);
CriticalSection::CriticalSection() {
    InitializeSRWLock(cast<::PSRWLOCK>(&m_srw_lock));
}
void CriticalSection::destroy() {}
void CriticalSection::lock() {
    AcquireSRWLockExclusive(cast<::PSRWLOCK>(&m_srw_lock));
}
bool CriticalSection::try_lock() {
    return TryAcquireSRWLockExclusive(cast<::PSRWLOCK>(&m_srw_lock)) != 0;
}
bool CriticalSection::try_lock_for(unsigned int) {
    return CriticalSection::try_lock();
}
void CriticalSection::unlock() {
    ReleaseSRWLockExclusive(cast<::PSRWLOCK>(&m_srw_lock));
}
internal::SRWLockPtr CriticalSection::native_handle() {
    return &m_srw_lock;
}
void CriticalSection::Create(CriticalSection* cs) {
    new (cs) CriticalSection;
}
Win32ConditionVariable::Win32ConditionVariable() {
    InitializeConditionVariable(cast<::PCONDITION_VARIABLE>(&m_condition_variable));
}
void Win32ConditionVariable::destroy() {}
void Win32ConditionVariable::wait(CriticalSection* lock) {
    if (!Win32ConditionVariable::wait_for(lock, INFINITE)) { abort_arlib(); }
}
bool Win32ConditionVariable::wait_for(CriticalSection* lock, unsigned int timeout) {
    return SleepConditionVariableSRW(
           cast<::PCONDITION_VARIABLE>(&m_condition_variable), cast<::PSRWLOCK>(lock->native_handle()), timeout, 0
           ) != 0;
}
void Win32ConditionVariable::notify_one() {
    WakeConditionVariable(cast<::PCONDITION_VARIABLE>(&m_condition_variable));
}
void Win32ConditionVariable::notify_all() {
    WakeAllConditionVariable(cast<::PCONDITION_VARIABLE>(&m_condition_variable));
}
void Win32ConditionVariable::Create(Win32ConditionVariable* cond) {
    new (cond) Win32ConditionVariable;
}
}    // namespace ARLib

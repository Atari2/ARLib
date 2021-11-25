#include "win_native_structs.h"
#include "../Assertion.h"

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>

namespace ARLib {

#define VERIFY_WIN32_STRUCT(win32_struct, arlib_struct)                                                                \
    static_assert(sizeof(win32_struct) == sizeof(arlib_struct),                                                        \
                  "Size of struct " #arlib_struct " is wrong when compared to the original " #win32_struct);           \
    static_assert(alignof(win32_struct) == alignof(arlib_struct),                                                      \
                  "Alignment of struct " #arlib_struct " is wrong when compared to the original " #win32_struct)

    VERIFY_WIN32_STRUCT(CRITICAL_SECTION, CriticalSection::CriticalSectionT);
    VERIFY_WIN32_STRUCT(LIST_ENTRY, CriticalSection::ListEntry);
    VERIFY_WIN32_STRUCT(CRITICAL_SECTION_DEBUG, CriticalSection::CriticalSectionDebugT);
    VERIFY_WIN32_STRUCT(SRWLOCK, CriticalSection::SRWLock);
    VERIFY_WIN32_STRUCT(CONDITION_VARIABLE, ConditionVariable::ConditionVariable);

    SyncApiModes SyncApiImplMode = SyncApiModes::Normal;

    namespace CriticalSection {

        Vista::Vista() { InitializeCriticalSectionEx(cast<::LPCRITICAL_SECTION>(&m_critical_section), 4000, 0); }
        void Vista::destroy() { DeleteCriticalSection(cast<::LPCRITICAL_SECTION>(&m_critical_section)); }

        void Vista::lock() { EnterCriticalSection(cast<::LPCRITICAL_SECTION>(&m_critical_section)); }

        bool Vista::try_lock() { return TryEnterCriticalSection(cast<::LPCRITICAL_SECTION>(&m_critical_section)) != 0; }

        bool Vista::try_lock_for(unsigned int) { return Vista::try_lock(); }

        void Vista::unlock() { LeaveCriticalSection(cast<::LPCRITICAL_SECTION>(&m_critical_section)); }

        CriticalSectionPtr Vista::native_handle() { return &m_critical_section; }

        Win7::Win7() { InitializeSRWLock(cast<::PSRWLOCK>(&m_srw_lock)); }

        void Win7::destroy() {}

        void Win7::lock() { AcquireSRWLockExclusive(cast<::PSRWLOCK>(&m_srw_lock)); }

        bool Win7::try_lock() { return TryAcquireSRWLockExclusive(cast<::PSRWLOCK>(&m_srw_lock)) != 0; }

        bool Win7::try_lock_for(unsigned int) { return Win7::try_lock(); }

        void Win7::unlock() { ReleaseSRWLockExclusive(cast<::PSRWLOCK>(&m_srw_lock)); }

        SRWLockPtr Win7::native_handle() { return &m_srw_lock; }

        void Create(Interface* p) {
#ifdef _CRT_WINDOWS
            new (p) Win7;
#else
            switch (SyncApiImplMode) {
            case SyncApiModes::Normal:
            case SyncApiModes::Win7:
                if (are_win7_sync_apis_available()) {
                    new (p) Win7;
                    return;
                }
            case SyncApiModes::Vista:
                new (p) Vista;
                return;
            default:
                abort_arlib();
            }
#endif
        }

    } // namespace CriticalSection

    namespace ConditionVariable {
        Vista::Vista() { InitializeConditionVariable(cast<::PCONDITION_VARIABLE>(&m_condition_variable)); }

        void Vista::destroy() {}

        void Vista::wait(CriticalSection::Interface* lock) {
            if (!Vista::wait_for(lock, INFINITE)) { abort_arlib(); }
        }

        bool Vista::wait_for(CriticalSection::Interface* lock, unsigned int timeout) {
            return SleepConditionVariableCS(
                   cast<::PCONDITION_VARIABLE>(&m_condition_variable),
                   cast<::LPCRITICAL_SECTION>(static_cast<CriticalSection::Vista*>(lock)->native_handle()),
                   timeout) != 0;
        }

        void Vista::notify_one() { WakeConditionVariable(cast<::PCONDITION_VARIABLE>(&m_condition_variable)); }

        void Vista::notify_all() { WakeAllConditionVariable(cast<::PCONDITION_VARIABLE>(&m_condition_variable)); }

        Win7::Win7() { InitializeConditionVariable(cast<::PCONDITION_VARIABLE>(&m_condition_variable)); }

        void Win7::destroy() {}

        void Win7::wait(CriticalSection::Interface* lock) {
            if (!Win7::wait_for(lock, INFINITE)) { abort_arlib(); }
        }

        bool Win7::wait_for(CriticalSection::Interface* lock, unsigned int timeout) {
            return SleepConditionVariableSRW(
                   cast<::PCONDITION_VARIABLE>(&m_condition_variable),
                   cast<::PSRWLOCK>(static_cast<CriticalSection::Win7*>(lock)->native_handle()), timeout, 0) != 0;
        }

        void Win7::notify_one() { WakeConditionVariable(cast<::PCONDITION_VARIABLE>(&m_condition_variable)); }

        void Win7::notify_all() { WakeAllConditionVariable(cast<::PCONDITION_VARIABLE>(&m_condition_variable)); }

        void Create(Interface* p) {
#ifdef _CRT_WINDOWS
            new (p) Win7;
#else
            switch (SyncApiImplMode) {
            case SyncApiModes::Normal:
            case SyncApiModes::Win7:
                if (CriticalSection::are_win7_sync_apis_available()) {
                    new (p) Win7;
                    return;
                }
            case SyncApiModes::Vista:
                new (p) Vista;
                return;
            default:
                abort_arlib();
            }
#endif
        }

    } // namespace ConditionVariable
} // namespace ARLib
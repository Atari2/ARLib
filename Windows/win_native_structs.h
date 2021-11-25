#pragma once
#include "../Compat.h"
#ifdef WINDOWS
#include "../Algorithm.h"
#include "../Conversion.h"
#include "../TypeTraits.h"

namespace ARLib {

    enum class SyncApiModes { Normal, Win7, Vista, ConCRT };
    // critical sections
    namespace CriticalSection {

        struct ListEntry {
            struct ListEntry* Flink;
            struct ListEntry* Blink;
        };

        struct CriticalSectionDebugT {
            unsigned short Type;
            unsigned short CreatorBackTraceIndex;
            struct CriticalSectionT* CriticalSection;
            ListEntry ProcessLocksList;
            unsigned long EntryCount;
            unsigned long ContentionCount;
            unsigned long Flags;
            unsigned short CreatorBackTraceIndexHigh;
            unsigned short SpareWORD;
        };

        struct CriticalSectionT {
            CriticalSectionDebugT* DebugInfo;
            long LockCount;
            long RecursionCount;
            void* OwningThread;
            void* LockSemaphore;
            unsigned long long SpinCount;
        };

        using CriticalSectionPtr = CriticalSectionT*;

        struct SRWLock {
            void* Ptr;
        };
        using SRWLockPtr = SRWLock*;

        class __declspec(novtable) Interface {
            public:
            virtual void lock() = 0;
            virtual bool try_lock() = 0;
            virtual bool try_lock_for(unsigned int) = 0;
            virtual void unlock() = 0;
            virtual void destroy() = 0;
        };

        class Vista final : public Interface {
            public:
            Vista();

            Vista(const Vista&) = delete;
            Vista& operator=(const Vista&) = delete;
            ~Vista() = delete;

            void destroy() override;

            void lock() override;

            bool try_lock() override;

            bool try_lock_for(unsigned int) override;

            void unlock() override;

            CriticalSectionPtr native_handle();

            private:
            CriticalSectionT m_critical_section;
        };

        class Win7 final : public Interface {
            public:
            Win7();

            ~Win7() = delete;
            Win7(const Win7&) = delete;
            Win7& operator=(const Win7&) = delete;

            void destroy() override;

            void lock() override;

            bool try_lock() override;

            bool try_lock_for(unsigned int) override;

            void unlock() override;

            SRWLockPtr native_handle();

            private:
            SRWLock m_srw_lock;
        };

        constexpr bool are_win7_sync_apis_available() { return true; }

        void Create(Interface* p);
    } // namespace CriticalSection

    namespace ConditionVariable {

        struct ConditionVariable {
            void* Ptr;
        };

        class __declspec(novtable) Interface {
            public:
            virtual void wait(CriticalSection::Interface*) = 0;
            virtual bool wait_for(CriticalSection::Interface*, unsigned int) = 0;
            virtual void notify_one() = 0;
            virtual void notify_all() = 0;
            virtual void destroy() = 0;
        };
        class Vista final : public Interface {
            public:
            Vista();

            ~Vista() = delete;
            Vista(const Vista&) = delete;
            Vista& operator=(const Vista&) = delete;

            void destroy() override;

            void wait(CriticalSection::Interface* lock) override;

            bool wait_for(CriticalSection::Interface* lock, unsigned int timeout) override;

            void notify_one() override;

            void notify_all() override;

            private:
            ConditionVariable m_condition_variable;
        };

        class Win7 final : public Interface {
            public:
            Win7();

            ~Win7() = delete;
            Win7(const Win7&) = delete;
            Win7& operator=(const Win7&) = delete;

            void destroy() override;

            void wait(CriticalSection::Interface* lock) override;

            bool wait_for(CriticalSection::Interface* lock, unsigned int timeout) override;

            void notify_one() override;

            void notify_all() override;

            private:
            ConditionVariable m_condition_variable;
        };

        void Create(Interface* p);
    } // namespace ConditionVariable

    constexpr size_t CriticalSectionSize = 64;
    constexpr size_t ConditionlVariableSize = 72;
    constexpr size_t CriticalSectionAlignment = 8;
    constexpr size_t ConditionVariableAlignment = 8;

    constexpr size_t CriticalSectionMaxSize =
    max_bt(max_bt(CriticalSectionSize, sizeof(CriticalSection::Vista)), sizeof(CriticalSection::Win7));
    constexpr size_t ConditionVariableMaxSize =
    max_bt(max_bt(ConditionlVariableSize, sizeof(ConditionVariable::Vista)), sizeof(ConditionVariable::Win7));
    constexpr size_t CriticalSectionMaxAlignment =
    max_bt(max_bt(CriticalSectionAlignment, alignof(CriticalSection::Vista)), alignof(CriticalSection::Win7));
    constexpr size_t ConditionVariableMaxAlignment =
    max_bt(max_bt(ConditionVariableAlignment, alignof(ConditionVariable::Vista)), alignof(ConditionVariable::Win7));

    using ThreadIdImplType = unsigned int;
    struct ThreadImplType {
        void* _Hnd;
        ThreadIdImplType _Id;
    };

    struct MutexInternalImplType {
        int type;
        AlignedStorageT<CriticalSectionMaxSize, CriticalSectionMaxAlignment> cs;
        long thread_id;
        int count;
        CriticalSection::Interface* _get_cs() { return reinterpret_cast<CriticalSection::Interface*>(&cs); }
    };

    struct CondInternalImplType {
        AlignedStorageT<ConditionVariableMaxSize, ConditionVariableMaxAlignment> cv;

        ConditionVariable::Interface* _get_cv() noexcept {
            return reinterpret_cast<ConditionVariable::Interface*>(&cv);
        }
    };

    using mutex_internal_imp_t = MutexInternalImplType;
    using cnd_internal_imp_t = CondInternalImplType;

} // namespace ARLib

#endif
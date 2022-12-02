#pragma once
#include "../Compat.h"
#ifdef WINDOWS
#include "../Algorithm.h"
#include "../Conversion.h"
#include "../TypeTraits.h"

namespace ARLib {

    namespace internal {
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

        struct ConditionVariable {
            void* Ptr;
        };
    } // namespace internal

    class CriticalSection {
        public:
        CriticalSection();

        ~CriticalSection() = delete;
        CriticalSection(const CriticalSection&) = delete;
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

    class ConditionVariable {
        public:
        ConditionVariable();

        ~ConditionVariable() = delete;
        ConditionVariable(const ConditionVariable&) = delete;
        ConditionVariable& operator=(const ConditionVariable&) = delete;

        void destroy();

        void wait(CriticalSection* lock);

        bool wait_for(CriticalSection* lock, unsigned int timeout);

        void notify_one();

        void notify_all();

        static void Create(ConditionVariable*);

        private:
        internal::ConditionVariable m_condition_variable;
    };

    using ThreadIdImplType = unsigned int;
    struct ThreadImplType {
        void* _Hnd;
        ThreadIdImplType _Id;
    };

    constexpr size_t CriticalSectionSize = 64;
    constexpr size_t ConditionlVariableSize = 72;
    constexpr size_t CriticalSectionAlignment = 8;
    constexpr size_t ConditionVariableAlignment = 8;

    constexpr size_t CriticalSectionMaxSize = max_bt(CriticalSectionSize, sizeof(CriticalSection));
    constexpr size_t ConditionVariableMaxSize = max_bt(ConditionlVariableSize, sizeof(ConditionVariable));
    constexpr size_t CriticalSectionMaxAlignment = max_bt(CriticalSectionAlignment, alignof(CriticalSection));
    constexpr size_t ConditionVariableMaxAlignment = max_bt(ConditionVariableAlignment, alignof(ConditionVariable));

    struct MutexInternalImplType {
        int type;
        AlignedStorageT<CriticalSectionMaxSize, CriticalSectionMaxAlignment> cs;
        long thread_id;
        int count;
        CriticalSection* _get_cs() { return reinterpret_cast<CriticalSection*>(&cs); }
    };

    struct CondInternalImplType {
        AlignedStorageT<ConditionVariableMaxSize, ConditionVariableMaxAlignment> cv;

        ConditionVariable* _get_cv() noexcept { return reinterpret_cast<ConditionVariable*>(&cv); }
    };

    using mutex_internal_imp_t = MutexInternalImplType;
    using cnd_internal_imp_t = CondInternalImplType;

} // namespace ARLib

#endif
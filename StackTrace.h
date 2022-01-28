#pragma once
#include "PrintInfo.h"

#ifdef DEBUG
#include "Types.h"

namespace ARLib {
    constexpr static unsigned short MAX_BACKTRACE = 512; // 512 frames should be enough for everything
    class BackTrace {
#ifdef ON_WINDOWS
        struct BackTraceGlobalState {
            void* process_handle;
            bool are_symbols_loaded;
            ~BackTraceGlobalState();
        };

        static BackTraceGlobalState s_state;
#endif
        char* m_backtrace_info[MAX_BACKTRACE]{};
        size_t m_size{0};

        public:
#ifdef ON_WINDOWS
        BackTrace(bool failing = false);
#else
        BackTrace() = default;
#endif
        static bool already_generating_backtrace;
        size_t size() const { return m_size; }
        const char* backtrace_at(size_t index) const { return m_backtrace_info[index]; }
        BackTrace(BackTrace&& other) noexcept;
        void append_frame(char* information, size_t info_len, bool need_to_alloc = true);
#ifdef ON_WINDOWS
        void* process_handle() { return s_state.process_handle; }
#else
        void* process_handle() {
            ASSERT_NOT_REACHED("Don't call BackTrace::process_handle on non-Windows systems");
            return nullptr;
        }
#endif
        ~BackTrace();
    };
    BackTrace capture_backtrace();
    void print_backtrace();

    template <>
    struct PrintInfo<BackTrace> {
        const BackTrace& m_trace;
        PrintInfo(const BackTrace& trace) : m_trace(trace) {}
        String repr() const;
    };

} // namespace ARLib
#define BACKTRACE()                                                                                                    \
    if (!ARLib::BackTrace::already_generating_backtrace) ARLib::print_backtrace()
#else
#define BACKTRACE()
#endif
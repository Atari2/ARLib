#pragma once

#ifdef DEBUG
#include "Types.h"

namespace ARLib {
    constexpr static unsigned short MAX_BACKTRACE = 512;        // 512 frames should be enough for everything
    class BackTrace {
        char* m_backtrace_info[MAX_BACKTRACE]{};
        size_t m_size{0};
        public:
        static bool already_generating_backtrace;
        size_t size() { return m_size; }
        const char* backtrace_at(size_t index) { return m_backtrace_info[index]; }
        BackTrace() = default;
        BackTrace(BackTrace&& other) noexcept;
        void append_frame(char* information, size_t info_len, bool need_to_alloc = true);
        ~BackTrace();
    };
    BackTrace capture_backtrace();
    void print_backtrace();
} // namespace ARLib
#define BACKTRACE() if (!ARLib::BackTrace::already_generating_backtrace) ARLib::print_backtrace()
#else
#define BACKTRACE()
#endif
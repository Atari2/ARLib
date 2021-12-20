#ifdef DEBUG
#include "StackTrace.h"
#include "Utility.h"
#include "cmath_compat.h"
#include "cstdio_compat.h"
#include "cstring_compat.h"

#ifdef ON_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
// clang-format no reorder these 2 headers
#include <dbghelp.h>
#else
#include "String.h"
#include "StringView.h"
#include <cstdlib> // this header is here literally just for std::free and I'm sad about it
#include <cxxabi.h>
#include <execinfo.h>
#endif
namespace ARLib {
    bool BackTrace::already_generating_backtrace = false;
    void BackTrace::append_frame(char* information, size_t info_len /* includes null terminator */,
                                 bool need_to_alloc) {
        if (need_to_alloc) {
            m_backtrace_info[m_size] = new char[info_len];
            strncpy(m_backtrace_info[m_size], information, info_len);
        } else {
            m_backtrace_info[m_size] = information;
        }
        m_size++;
    }
    BackTrace::BackTrace(BackTrace&& other) noexcept {
        m_size = other.m_size;
        other.m_size = 0;
        memcpy(m_backtrace_info, other.m_backtrace_info, m_size * sizeof(char*));
    }
    BackTrace::~BackTrace() {
        for (size_t i = 0; i < m_size; i++) {
            delete[] m_backtrace_info[i];
        }
    }
    void print_backtrace() {
        if (BackTrace::already_generating_backtrace) {
            ARLib::puts(
            "Trying to construct a backtrace while another backtrace is already being generated, aborting...");
        }
        BackTrace::already_generating_backtrace = true;
        BackTrace btrace = capture_backtrace();
        puts("---- BACKTRACE START ----");
        for (size_t i = 0; i < btrace.size(); i++) {
            printf("\t%s\n", btrace.backtrace_at(i));
        }
        puts("---- BACKTRACE END ----");
    }
#ifdef ON_WINDOWS
    BackTrace capture_backtrace() {
        BackTrace trace{};
        constexpr DWORD frames_to_skip = 1;                // skip the frame from this function, it's not needed
        constexpr DWORD frames_to_capture = MAX_BACKTRACE; // we get as many frames as we can
        PVOID* backtrace = new PVOID[frames_to_capture];
        auto hdl = GetCurrentProcess();
        if (!SymInitialize(hdl, nullptr, true)) {
            char err[]{"Failed to initialize symbols"};
            trace.append_frame(err, sizeof_array(err));
            return trace;
        }
        SymSetOptions(SYMOPT_LOAD_LINES);
        auto res = RtlCaptureStackBackTrace(frames_to_skip, frames_to_capture, backtrace, nullptr);
        for (auto i = 0; i < res; ++i) {
            constexpr size_t ptr_fmt_len = 15; // how many chars to print a pointer on MSVC (e.g. `00007FFBED00703`)
            char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)]{};
            PSYMBOL_INFO psymbol = reinterpret_cast<PSYMBOL_INFO>(buffer);
            psymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            psymbol->MaxNameLen = MAX_SYM_NAME;
            if (!SymFromAddr(hdl, reinterpret_cast<DWORD64>(backtrace[i]), nullptr, psymbol)) {
                constexpr size_t len = sizeof_array("Failed to retrieve debug info for address %p") + ptr_fmt_len;
                char message[len]{};
                snprintf(message, len, "Failed to retrieve debug info for address %p", backtrace[i]);
                trace.append_frame(message, len);
            } else {
                IMAGEHLP_LINE64 line{};
                DWORD displ = 0;
                line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
                if (!SymGetLineFromAddr64(hdl, reinterpret_cast<DWORD64>(backtrace[i]), &displ, &line)) {
                    constexpr size_t len =
                    sizeof_array("Failed to retrieve debug info for line at address %p") + ptr_fmt_len;
                    char message[len]{};
                    snprintf(message, len, "Failed to retrieve debug info for line at address %p", backtrace[i]);
                    trace.append_frame(message, len);
                }
                constexpr size_t len = sizeof_array("`%s` in %s at [%d]:[%d]");
                size_t number_len =
                line.LineNumber > 0 ? static_cast<size_t>(ARLib::log10(static_cast<double>(line.LineNumber))) + 1 : 1;
                size_t displ_len = displ > 0 ? static_cast<size_t>(ARLib::log10(static_cast<double>(displ))) + 1 : 1;
                size_t total_length = psymbol->NameLen + ARLib::strlen(line.FileName) + number_len + displ_len + len;
                char* backtrace_msg = new char[total_length];
                snprintf(backtrace_msg, total_length, "`%s` in %s at [%d]:[%d]", psymbol->Name, line.FileName,
                         line.LineNumber, displ);
                trace.append_frame(backtrace_msg, total_length, false);
            }
        }
        return trace;
    }
#else
    BackTrace capture_backtrace() {
        BackTrace trace{};
        constexpr int frames_to_capture = MAX_BACKTRACE;
        void* buffer[frames_to_capture]{};
        int ret = backtrace(buffer, frames_to_capture);
        char** symbols = backtrace_symbols(buffer, ret);
        for (int i = 0; i < ret; i++) {
            int status = 0;
            size_t len = strlen(symbols[i]);
            char* start_of_name = nullptr;
            char* end_of_name = nullptr;
            for (size_t j = 0; j < len; j++) {
                if (symbols[i][j] == '(')
                    start_of_name = &symbols[i][j] + 1;
                else if (symbols[i][j] == '+')
                    end_of_name = &symbols[i][j];
                if (end_of_name && start_of_name) break;
            }
            if (!start_of_name) start_of_name = symbols[i];
            if (!end_of_name) end_of_name = symbols[i] + len;
            String name_to_demangle{start_of_name, end_of_name};
            char* demangled = abi::__cxa_demangle(name_to_demangle.data(), nullptr, nullptr, &status);
            if (status != 0) {
                trace.append_frame(symbols[i], strlen(symbols[i]) + 1, true);
            } else {
                String demangled_full_symbol{symbols[i], len};
                demangled_full_symbol.ireplace(name_to_demangle.data(), demangled, 1);
                size_t sz = demangled_full_symbol.size();
                trace.append_frame(demangled_full_symbol.release(), sz + 1);
            }
        }
        std::free(symbols);
        return trace;
    }
#endif
} // namespace ARLib
#endif
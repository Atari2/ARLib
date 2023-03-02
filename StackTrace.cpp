#ifdef DEBUG
    #include "StackTrace.h"
    #include "CharConvHelpers.h"
    #include "Utility.h"
    #include "cmath_compat.h"
    #include "cstdio_compat.h"
    #include "cstring_compat.h"
    #include "String.h"

    #ifdef ON_WINDOWS
        #include <windows.h>
        // clang-format no reorder these 2 headers
        #include "Threading.h"
        #include "arlib_osapi.h"
        #include <dbghelp.h>
    #else
        #include "String.h"
        #include "StringView.h"
        #include <cstdlib>    // this header is here literally just for std::free and I'm sad about it
        #include <cxxabi.h>
        #include <execinfo.h>
    #endif
namespace ARLib {
    #ifdef ON_WINDOWS
BackTrace::BackTraceGlobalState BackTrace::s_state = BackTrace::BackTraceGlobalState{ nullptr, false };
BackTrace::BackTrace(bool failing) {
    if (failing) [[unlikely]]
        return;
    if (!s_state.are_symbols_loaded) {
        auto proc = GetCurrentProcess();
        s_state.are_symbols_loaded =
        DuplicateHandle(proc, proc, proc, &s_state.process_handle, PROCESS_QUERY_INFORMATION, false, 0) &&
        SymInitialize(s_state.process_handle, nullptr, true);
        if (!s_state.are_symbols_loaded) {
            s_state.process_handle = nullptr;
            char err[]{ "Failed to initialize symbols" };
            append_frame(err, sizeof_array(err));
        }
        SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_ALLOW_ABSOLUTE_SYMBOLS);
    }
}
    #endif

bool BackTrace::already_generating_backtrace = false;
void BackTrace::append_frame(char* information, size_t info_len /* includes null terminator */, bool need_to_alloc) {
    if (need_to_alloc) {
        m_backtrace_info[m_size] = new char[info_len];
        strncpy(m_backtrace_info[m_size], information, info_len);
    } else {
        m_backtrace_info[m_size] = information;
    }
    m_size++;
}
BackTrace::BackTrace(BackTrace&& other) noexcept {
    m_size       = other.m_size;
    other.m_size = 0;
    memcpy(m_backtrace_info, other.m_backtrace_info, m_size * sizeof(char*));
}
BackTrace::~BackTrace() {
    for (size_t i = 0; i < m_size; i++) { delete[] m_backtrace_info[i]; }
}
void print_backtrace() {
    if (BackTrace::already_generating_backtrace) {
        ARLib::puts("Trying to construct a backtrace while another backtrace is already being generated, aborting...");
        return;
    }
    BackTrace btrace = capture_backtrace();
    puts("---- BACKTRACE START ----");
    for (size_t i = 0; i < btrace.size(); i++) { printf("\t%s\n", btrace.backtrace_at(i)); }
    puts("---- BACKTRACE END ----");
}
    #ifdef ON_WINDOWS
BackTrace capture_backtrace() {
    if (BackTrace::already_generating_backtrace) {
        ARLib::puts("Trying to construct a backtrace while another backtrace is already being generated, aborting...");
        return BackTrace{ true };
    }
    Mutex mtx{};
    ScopedLock lock{ mtx };
    BackTrace::already_generating_backtrace = true;
    BackTrace trace{};
    if (trace.process_handle() == nullptr) {
        BackTrace::already_generating_backtrace = false;
        return trace;
    }
    constexpr DWORD frames_to_skip    = 1;                // skip the frame from this function, it's not needed
    constexpr DWORD frames_to_capture = MAX_BACKTRACE;    // we get as many frames as we can
    PVOID backtrace[frames_to_capture];
    auto res = RtlCaptureStackBackTrace(frames_to_skip, frames_to_capture, backtrace, nullptr);
    for (auto i = 0; i < res; ++i) {
        constexpr size_t ptr_fmt_len = 15;    // how many chars to print a pointer on MSVC (e.g. `00007FFBED00703`)
        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)]{};
        PSYMBOL_INFO psymbol  = reinterpret_cast<PSYMBOL_INFO>(buffer);
        psymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        psymbol->MaxNameLen   = MAX_SYM_NAME;
        if (!SymFromAddr(trace.process_handle(), reinterpret_cast<DWORD64>(backtrace[i]), nullptr, psymbol)) {
            constexpr size_t len =
            sizeof_array("Failed to retrieve debug info for address %p, error was %s") + ptr_fmt_len;
            String error  = last_error();
            char* message = new char[len + error.size()];
            snprintf(
            message, len + error.size(), "Failed to retrieve debug info for address %p, error was %s", backtrace[i],
            error.data()
            );
            trace.append_frame(message, len, false);
        } else {
            IMAGEHLP_LINE64 line{};
            DWORD displ       = 0;
            line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
            if (!SymGetLineFromAddr64(trace.process_handle(), reinterpret_cast<DWORD64>(backtrace[i]), &displ, &line)) {
                constexpr size_t len =
                sizeof_array("Failed to retrieve line info for `%s` at address %p, error was \"%s\"") + ptr_fmt_len;
                String error = last_error();
                error.itrim();
                size_t total_len = psymbol->NameLen + len + error.size();
                char* message    = new char[total_len];
                snprintf(
                message, total_len, "Failed to retrieve line info for `%s` at address %p, error was \"%s\"",
                psymbol->Name, backtrace[i], error.data()
                );
                trace.append_frame(message, len, false);
            } else {
                constexpr size_t len = sizeof_array("%s@%s[%d:%d]");
                size_t number_len    = StrLenFromIntegral<10>(line.LineNumber);
                size_t displ_len     = StrLenFromIntegral<10>(displ);
                size_t total_length  = psymbol->NameLen + ARLib::strlen(line.FileName) + number_len + displ_len + len;
                char* backtrace_msg  = new char[total_length];
                snprintf(
                backtrace_msg, total_length, "%s@%s[%d:%d]", line.FileName, psymbol->Name, line.LineNumber, displ
                );
                trace.append_frame(backtrace_msg, total_length, false);
            }
        }
    }
    BackTrace::already_generating_backtrace = false;
    return trace;
}
    #else
BackTrace capture_backtrace() {
    BackTrace trace{};
    constexpr int frames_to_capture = MAX_BACKTRACE;
    void* buffer[frames_to_capture]{};
    int ret        = backtrace(buffer, frames_to_capture);
    char** symbols = backtrace_symbols(buffer, ret);
    for (int i = 0; i < ret; i++) {
        int status          = 0;
        size_t len          = strlen(symbols[i]);
        char* start_of_name = nullptr;
        char* end_of_name   = nullptr;
        for (size_t j = 0; j < len; j++) {
            if (symbols[i][j] == '(')
                start_of_name = &symbols[i][j] + 1;
            else if (symbols[i][j] == '+')
                end_of_name = &symbols[i][j];
            if (end_of_name && start_of_name) break;
        }
        if (!start_of_name) start_of_name = symbols[i];
        if (!end_of_name) end_of_name = symbols[i] + len;
        String name_to_demangle{ start_of_name, end_of_name };
        char* demangled = abi::__cxa_demangle(name_to_demangle.data(), nullptr, nullptr, &status);
        if (status != 0) {
            trace.append_frame(symbols[i], strlen(symbols[i]) + 1, true);
        } else {
            String demangled_full_symbol{ symbols[i], len };
            demangled_full_symbol.ireplace(name_to_demangle.data(), demangled, 1);
            size_t sz = demangled_full_symbol.size();
            trace.append_frame(demangled_full_symbol.release(), sz + 1, false);
        }
    }
    std::free(symbols);
    return trace;
}
    #endif
    #ifdef ON_WINDOWS
BackTrace::BackTraceGlobalState::~BackTraceGlobalState() {
    if (are_symbols_loaded && process_handle) { SymCleanup(process_handle); }
}
    #endif
String PrintInfo<BackTrace>::repr() const {
    String backtrace{ "---- BACKTRACE START ----\n" };
    for (size_t i = 0; i < m_trace.size(); i++) {
        backtrace += '\t';
        backtrace += m_trace.backtrace_at(i);
        backtrace += '\n';
    }
    backtrace += "---- BACKTRACE END ----"_s;
    return backtrace;
}
}    // namespace ARLib
#endif

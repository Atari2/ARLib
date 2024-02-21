#include "Windows/win_native_process.hpp"
#include <Windows.h>
#include <TlHelp32.h>
namespace ARLib {

MAKE_BITFIELD_ENUM(Win32ProcessFlags)
Win32Process::Win32Process(StringView command) {
    m_s_info.cb      = sizeof(STARTUPINFO);
    m_cmdline_buffer = new char[32767];
    memcpy(m_cmdline_buffer, command.data(), command.size());
    m_cmdline_buffer[command.size()] = '\0';
    m_proc_name                      = command.str();
}
Win32Process& Win32Process::with_timeout(size_t ms_timeout) {
    m_timeout = static_cast<timeout_t>(ms_timeout);
    return *this;
}
Win32Process& Win32Process::with_args(std::initializer_list<ArgumentString> args) {
    size_t proc_name_len = m_proc_name.size();
    for (const auto& arg : args) {
        m_cmdline_buffer[proc_name_len]     = ' ';
        m_cmdline_buffer[proc_name_len + 1] = '\0';
        auto argument                       = arg.argument();
        strcat(m_cmdline_buffer, argument.data());
        proc_name_len += argument.size() + 1;
    }
    return *this;
}
ProcessResult Win32Process::suspend() {
    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    THREADENTRY32 threadEntry{};
    threadEntry.dwSize = sizeof(THREADENTRY32);
    if (!Thread32First(hThreadSnap, &threadEntry)) return get_last_error();
    do {
        if (threadEntry.th32OwnerProcessID == m_p_info.dwProcessId) {
            HANDLE hthread = OpenThread(THREAD_ALL_ACCESS, FALSE, threadEntry.th32ThreadID);
            if (hthread == INVALID_HANDLE_VALUE || hthread == nullptr) continue;
            if (SuspendThread(hthread) == (DWORD)-1) { return get_last_error(); }
            if (!CloseHandle(hthread)) return get_last_error();
        }
    } while (Thread32Next(hThreadSnap, &threadEntry));
    if (!CloseHandle(hThreadSnap)) return get_last_error();
    return {};
}
ProcessResult Win32Process::resume() {
    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    THREADENTRY32 threadEntry{};
    threadEntry.dwSize = sizeof(THREADENTRY32);
    if (!Thread32First(hThreadSnap, &threadEntry)) return get_last_error();
    do {
        if (threadEntry.th32OwnerProcessID == m_p_info.dwProcessId) {
            HANDLE hthread = OpenThread(THREAD_ALL_ACCESS, FALSE, threadEntry.th32ThreadID);
            if (hthread == INVALID_HANDLE_VALUE || hthread == nullptr) continue;
            if (ResumeThread(hthread) == (DWORD)-1) { return get_last_error(); }
            if (!CloseHandle(hthread)) return get_last_error();
        }
    } while (Thread32Next(hThreadSnap, &threadEntry));
    if (!CloseHandle(hThreadSnap)) return get_last_error();
    return {};
}
bool Win32Process::terminate(exit_code_t exit_code) {
    m_exit_code = exit_code;
    return TerminateProcess(m_p_info.hProcess, exit_code);
}
const OutputType& Win32Process::output() const {
    return m_output;
}
const OutputType& Win32Process::error() const {
    return m_error;
}
ProcessResult Win32Process::wait_for_input(size_t ms_timeout) {
    GUITHREADINFO info{};
    info.cbSize = sizeof(GUITHREADINFO);
    if (GetGUIThreadInfo(m_p_info.dwThreadId, &info)) {
        switch (WaitForInputIdle(m_p_info.hProcess, static_cast<DWORD>(ms_timeout))) {
            case WAIT_FAILED:
                return get_last_error();
            case WAIT_TIMEOUT:
                return "Timeout expired"_s;
            default:
                return {};
        }
    } else {
        return "Cannot wait for input on processes without Message queue"_s;
    }
}
ProcessResult Win32Process::write_input(StringView input, Win32Process::completion_routine_t routine) {
    if (!m_redirected_stdin) return "To write to a process' input without pipes or files just use stdin"_s;
    DWORD dwWritten{};
    BOOL bSuccess = FALSE;
    HANDLE hdl    = choose_handle(Win32PipeType::Input);
    if (routine != nullptr) {
        LPOVERLAPPED overlapped = new OVERLAPPED;
        ZeroMemory(overlapped, sizeof(OVERLAPPED));
        m_overlapped.push_back(cast<internal::LPOverlapped>(overlapped));
        bSuccess = WriteFileEx(
        hdl, input.data(), static_cast<DWORD>(input.size()), overlapped, cast<LPOVERLAPPED_COMPLETION_ROUTINE>(routine)
        );
        // the SleepEx is here to set the thread state to Alertable
        SleepEx(0, TRUE);
        if (bSuccess == FALSE) { return get_last_error(); }
    } else {
        bSuccess = WriteFile(hdl, input.data(), static_cast<DWORD>(input.size()), &dwWritten, NULL);
        if (dwWritten != input.size() || bSuccess == FALSE) { return get_last_error(); }
        if (auto ret = flush_input(); ret.is_error()) { return ret.to_error(); };
    }
    return {};
}
ProcessResult Win32Process::write_data(const ReadOnlyView<uint8_t>& data, Win32Process::completion_routine_t routine) {
    if (!m_redirected_stdin) return "To write to a process' input without pipes or files just use stdin"_s;
    DWORD dwWritten{};
    BOOL bSuccess = FALSE;
    HANDLE hdl    = choose_handle(Win32PipeType::Input);
    if (routine != nullptr) {
        LPOVERLAPPED overlapped = new OVERLAPPED;
        ZeroMemory(overlapped, sizeof(OVERLAPPED));
        m_overlapped.push_back(cast<internal::LPOverlapped>(overlapped));
        bSuccess = WriteFileEx(
        hdl, data.data(), static_cast<DWORD>(data.size()), overlapped, cast<LPOVERLAPPED_COMPLETION_ROUTINE>(routine)
        );
        SleepEx(0, TRUE);
        if (bSuccess == FALSE) { return get_last_error(); }
    } else {
        bSuccess = WriteFile(hdl, data.data(), static_cast<DWORD>(data.size()), &dwWritten, NULL);
        if (dwWritten != data.size() || bSuccess == FALSE) { return get_last_error(); }
        if (auto ret = flush_input(); ret.is_error()) { return ret.to_error(); };
    }
    return {};
}
ProcessResult Win32Process::set_pipe(Win32PipeType type) {
    constexpr static Array<StringView, 3> pipe_names{
        R"(\\.\pipe\libproc_stdout)",
        R"(\\.\pipe\libproc_stdin)",
        R"(\\.\pipe\libproc_stderr)",
    };
    SECURITY_ATTRIBUTES sa{};
    sa.nLength              = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle       = TRUE;
    sa.lpSecurityDescriptor = NULL;
    m_s_info.dwFlags |= STARTF_USESTDHANDLES;
    HANDLE pipe = CreateNamedPipeA(
    pipe_names[static_cast<int>(type)].data(), PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
    PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, 4096, 4096, 0, NULL
    );
    HANDLE pipe_hdl = CreateFileA(
    pipe_names[static_cast<int>(type)].data(), Win32PipeType::Input == type ? GENERIC_READ : GENERIC_WRITE,
    Win32PipeType::Input == type ? FILE_SHARE_READ : FILE_SHARE_WRITE, &sa, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL
    );
    HANDLE pipe_rd = Win32PipeType::Input == type ? pipe_hdl : pipe;
    HANDLE pipe_wr = Win32PipeType::Input == type ? pipe : pipe_hdl;
    if (pipe_rd == INVALID_HANDLE_VALUE || pipe_wr == INVALID_HANDLE_VALUE) { return get_last_error(); }
    switch (type) {
        case Win32PipeType::Output:
            m_pipes.output      = pipe_rd;
            m_s_info.hStdOutput = pipe_wr;
            setup_output_reader();
            m_redirected_stdout = true;
            break;
        case Win32PipeType::Input:
            m_pipes.input      = pipe_wr;
            m_s_info.hStdInput = pipe_rd;
            m_redirected_stdin = true;
            break;
        case Win32PipeType::Error:
            m_pipes.error      = pipe_rd;
            m_s_info.hStdError = pipe_wr;
            setup_error_reader();
            m_redirected_stderr = true;
            break;
    }
    m_internal_handles.push_back(pipe_hdl);
    m_internal_handles.push_back(pipe);
    if (type == Win32PipeType::Error || type == Win32PipeType::Output)
        SetHandleInformation(pipe_rd, HANDLE_FLAG_INHERIT, 0);
    else
        SetHandleInformation(pipe_wr, HANDLE_FLAG_INHERIT, 0);
    return {};
}
void Win32Process::stop_and_join_thread(JThread& t) {
    if (t.joinable()) {
        t.request_stop();
        t.join();
    }
}
ProcessResult Win32Process::close_pipe(Win32PipeType type) {
    BOOL ret_val = 0;
    bool redirects[3]{ m_redirected_stdout, m_redirected_stdin, m_redirected_stderr };
    if (!redirects[from_enum(type)]) { return "Pipe is already closed"_s; }
    switch (type) {
        case Win32PipeType::Input:
            ret_val = DisconnectNamedPipe(m_pipes.input);
            break;
        case Win32PipeType::Output:
            stop_and_join_thread(m_output_reader_thread);
            ret_val = DisconnectNamedPipe(m_pipes.output);
            break;
        case Win32PipeType::Error:
            stop_and_join_thread(m_error_reader_thread);
            ret_val = DisconnectNamedPipe(m_pipes.error);
            break;
    }
    if (!ret_val) { return "Cannot close pipe"_s; }
    return {};
}
ProcessResult Win32Process::flush_input() {
    if (!m_redirected_stdin) return {};
    auto hdl = choose_handle(Win32PipeType::Input);
    if (!FlushFileBuffers(hdl)) { return "Couldn't flush file buffer"_s; }
    return {};
}
Win32Process& Win32Process::with_cwd(StringView cwd) {
    m_working_dir = String{ cwd };
    return *this;
}
Win32Process& Win32Process::with_env(std::initializer_list<EnvironMapString> env_values) {
    bool unicode_process = false;
    if ((m_dwflags & Win32ProcessFlags::UnicodeEnvironment) != Win32ProcessFlags::None) {
        unicode_process = true;
        m_env_buffer.set<wchar_t*>(nullptr);
    } else {
        unicode_process = false;
        m_env_buffer.set<char*>(nullptr);
    }
    for (const auto& env_value : env_values) m_environment.insert(env_value);

    if (unicode_process) {
        wchar_t*& env_buffer = get<wchar_t*>(m_env_buffer);
        if (m_environment.size() > 0) {
            if (env_buffer != nullptr) delete[] env_buffer;
            size_t full_size = sum(
                               m_environment.begin(), m_environment.end(),
                               [](const EnvironMapString& v) {
                                   const auto& [name, val] = v;
                                   return (name.size() + 1) + 1 + (val.size() + 1); /*len+null+'='+len+null*/
                               }
                               ) +
                               1;
            env_buffer = new wchar_t[full_size];
            memset(env_buffer, 0, full_size);
            size_t idx = 0;
            for (const auto& [vname, vval] : m_environment) {
                const auto& name = vname.wstring();
                const auto& val  = vval.wstring();
                memcpy(env_buffer + idx, name.data(), name.size() * sizeof(wchar_t));
                idx += name.size();
                env_buffer[idx++] = L'=';
                memcpy(env_buffer + idx, val.data(), val.size() * sizeof(wchar_t));
                idx += val.size();
                env_buffer[idx++] = L'\0';
            }
            env_buffer[idx] = L'\0';
        }
    } else {
        char*& env_buffer = get<char*>(m_env_buffer);
        if (m_environment.size() > 0) {
            if (env_buffer != nullptr) delete[] env_buffer;
            size_t full_size = sum(
                               m_environment.begin(), m_environment.end(),
                               [](const EnvironMapString& v) {
                                   const auto& [name, val] = v;
                                   return (name.size() + 1) + 1 + (name.size() + 1); /*len+null+'='+len+null*/
                               }
                               ) +
                               1;
            env_buffer = new char[full_size];
            memset(env_buffer, 0, full_size);
            size_t idx = 0;
            for (const auto& [vname, vval] : m_environment) {
                const auto& name = vname.string();
                const auto& val  = vval.string();
                memcpy(env_buffer + idx, name.data(), name.size() * sizeof(char));
                idx += name.size();
                env_buffer[idx++] = '=';
                memcpy(env_buffer + idx, val.data(), val.size() * sizeof(char));
                idx += val.size();
                env_buffer[idx++] = '\0';
            }
            env_buffer[idx] = '\0';
        }
    }
    return *this;
}
Win32Process& Win32Process::with_flag(Win32ProcessFlags flag) {
    m_dwflags = m_dwflags | flag;
    return *this;
}
Win32Process& Win32Process::set_blocking() {
    m_launch_waits_for_exit = true;
    return *this;
}
Win32Process& Win32Process::set_wait_at_destruction(bool wait) {
    m_wait_on_dtor = wait;
    return *this;
}
static void clean_handle(HANDLE pipe) {
    BOOL bSuccess = FALSE;
    DWORD bytesAvailable{};
    DWORD dwRead{};
    if (PeekNamedPipe(pipe, NULL, 0, NULL, &bytesAvailable, NULL) && bytesAvailable != 0) {
        CharPtr chBuf{ bytesAvailable + 1 };
        bSuccess = ReadFile(pipe, chBuf.ptr, bytesAvailable, &dwRead, NULL);
        if (!bSuccess || dwRead == 0 || dwRead > bytesAvailable) return;
    }
}
void Win32Process::setup_exit_handler() {
    m_exit_handler_thread = JThread{ [this](StopToken stoken) {
        BOOL ret          = GetExitCodeProcess(m_p_info.hProcess, &m_exit_code);
        while (ret && m_exit_code == STILL_ACTIVE && !stoken.stop_requested()) {
            // poll every 10 ms
            Sleep(10);
            ret = GetExitCodeProcess(m_p_info.hProcess, &m_exit_code);
        }
        // we don't call on_exit if the thread was asked to be stopped
        // because this call wouldn't be "real"
        if (!stoken.stop_requested()) {
            on_exit(m_exit_code);
            // unregister the callback
            on_exit.unregister();
        }
        // on exit, we read everything that the process has left on input
        // since otherwise FlushFileBuffers will hang forever
        clean_handle(m_s_info.hStdInput);
    } };
}
HANDLE Win32Process::choose_handle(Win32PipeType type) {
    // output = 0
    // input = 1
    // error = 2
    HANDLE handles[3]{};
    bool redirects[3]{ m_redirected_stdout, m_redirected_stdin, m_redirected_stderr };
    if (redirects[from_enum(type)]) {
        handles[from_enum(Win32PipeType::Output)] = m_pipes.output;
        handles[from_enum(Win32PipeType::Input)]  = m_pipes.input;
        handles[from_enum(Win32PipeType::Error)]  = m_pipes.error;
    } else {
        handles[from_enum(Win32PipeType::Output)] = GetStdHandle(STD_OUTPUT_HANDLE);
        handles[from_enum(Win32PipeType::Input)]  = GetStdHandle(STD_INPUT_HANDLE);
        handles[from_enum(Win32PipeType::Error)]  = GetStdHandle(STD_ERROR_HANDLE);
    }
    return handles[from_enum(type)];
}
Win32Process::Win32Process(Win32Process&& proc) noexcept :
    m_pipes(proc.m_pipes), m_output(move(proc.m_output)), m_error(move(proc.m_error)),
    m_working_dir(move(proc.m_working_dir)), m_environment(move(proc.m_environment)),
    m_internal_handles(move(proc.m_internal_handles)), m_overlapped(move(proc.m_overlapped)), m_p_info(proc.m_p_info),
    m_s_info(proc.m_s_info), m_proc_name(move(proc.m_proc_name)), m_cmdline_buffer(proc.m_cmdline_buffer),
    m_wait_on_dtor(proc.m_wait_on_dtor), m_launched(proc.m_launched),
    m_launch_waits_for_exit(proc.m_launch_waits_for_exit), m_timeout(proc.m_timeout), m_exit_code(proc.m_exit_code),
    m_dwflags(proc.m_dwflags), m_redirected_stdin(proc.m_redirected_stdin),
    m_redirected_stdout(proc.m_redirected_stdout), m_redirected_stderr(proc.m_redirected_stderr),
    m_output_reader_thread(move(proc.m_output_reader_thread)), m_error_reader_thread(move(proc.m_error_reader_thread)),
    m_exit_handler_thread(move(proc.m_exit_handler_thread)), on_output(move(proc.on_output)),
    on_error(move(proc.on_error)), on_exit(move(proc.on_exit)) {
    proc.m_cmdline_buffer = nullptr;
    proc.m_launched       = false;
    m_env_buffer          = proc.m_env_buffer;
    if ((proc.m_dwflags & Win32ProcessFlags::UnicodeEnvironment) == Win32ProcessFlags::UnicodeEnvironment) {
        proc.m_env_buffer = static_cast<wchar_t*>(nullptr);
    } else {
        proc.m_env_buffer = static_cast<char*>(nullptr);
    }
}
Win32Process& Win32Process::operator=(Win32Process&& proc) noexcept {
    m_pipes                 = proc.m_pipes;
    m_output                = move(proc.m_output);
    m_error                 = move(proc.m_error);
    m_working_dir           = move(proc.m_working_dir);
    m_environment           = move(proc.m_environment);
    m_internal_handles      = move(proc.m_internal_handles);
    m_overlapped            = move(proc.m_overlapped);
    m_p_info                = proc.m_p_info;
    m_s_info                = proc.m_s_info;
    m_proc_name             = move(proc.m_proc_name);
    m_cmdline_buffer        = proc.m_cmdline_buffer;
    m_wait_on_dtor          = proc.m_wait_on_dtor;
    m_launched              = proc.m_launched;
    m_launch_waits_for_exit = proc.m_launch_waits_for_exit;
    m_timeout               = proc.m_timeout;
    m_exit_code             = proc.m_exit_code;
    m_dwflags               = proc.m_dwflags;
    m_redirected_stdin      = proc.m_redirected_stdin;
    m_redirected_stdout     = proc.m_redirected_stdout;
    m_redirected_stderr     = proc.m_redirected_stderr;
    m_output_reader_thread  = move(proc.m_output_reader_thread);
    m_error_reader_thread   = move(proc.m_error_reader_thread);
    m_exit_handler_thread   = move(proc.m_exit_handler_thread);
    on_output               = move(proc.on_output);
    on_error                = move(proc.on_error);
    on_exit                 = move(proc.on_exit);
    proc.m_cmdline_buffer   = nullptr;
    proc.m_launched         = false;
    m_env_buffer            = proc.m_env_buffer;
    if ((proc.m_dwflags & Win32ProcessFlags::UnicodeEnvironment) == Win32ProcessFlags::UnicodeEnvironment) {
        proc.m_env_buffer = static_cast<wchar_t*>(nullptr);
    } else {
        proc.m_env_buffer = static_cast<char*>(nullptr);
    }
    return *this;
}
void Win32Process::peek_and_read_pipe(WinHandle pipe) {
    BOOL bSuccess = FALSE;
    DWORD bytesAvailable{};
    DWORD dwRead{};
    ReadOnlyView<uint8_t> data{ (const uint8_t*)nullptr, 0ull };
    if (PeekNamedPipe(pipe, NULL, 0, NULL, &bytesAvailable, NULL) && bytesAvailable != 0) {
        CharPtr chBuf{ bytesAvailable + 1 };
        bSuccess = ReadFile(pipe, chBuf.ptr, bytesAvailable, &dwRead, NULL);
        if (!bSuccess || dwRead == 0 || dwRead > bytesAvailable) return;
        data              = ReadOnlyView<uint8_t>{ reinterpret_cast<uint8_t*>(chBuf.ptr), dwRead };
        chBuf.ptr[dwRead] = '\0';
        if (pipe == m_pipes.error) {
            on_error(OutputType{ data });
            m_error.append(data);
        } else {
            on_output(OutputType{ data });
            m_output.append(data);
        }
    }
}
Result<exit_code_t, Error> Win32Process::wait_for_exit() {
    if (!m_launched) return exit_code_t{ EXIT_FAILURE };

    // stop the exit handler thread, we're about to check for exit ourselves anyway
    m_exit_handler_thread.request_stop();

    DWORD wait_result{};
    if (GetExitCodeProcess(m_p_info.hProcess, &m_exit_code)) {
        if (m_exit_code == STILL_ACTIVE) { wait_result = WaitForSingleObject(m_p_info.hProcess, m_timeout); }
    } else {
        return get_last_error();
    }

    GetExitCodeProcess(m_p_info.hProcess, &m_exit_code);

    // call the exit handler (if this was already called on the exit handler thread, the callback will be null
    on_exit(m_exit_code);

    // if the wait has resulted in a timeout we kill the process with failure exit code
    if (wait_result == WAIT_TIMEOUT) {
        // the wait has timeouted, but terminate didn't work either
        // idk? panico? print last error and go on
        if (!terminate(EXIT_FAILURE)) return get_last_error();
    }
    stop_and_join_thread(m_output_reader_thread);
    stop_and_join_thread(m_error_reader_thread);
    if (m_redirected_stdout) FlushFileBuffers(choose_handle(Win32PipeType::Output));
    if (m_redirected_stderr) FlushFileBuffers(choose_handle(Win32PipeType::Error));
    // the exit handler will stop by itself
    for (HANDLE hdl : m_internal_handles) { CloseHandle(hdl); }
    m_internal_handles.clear();
    CloseHandle(m_p_info.hProcess);
    CloseHandle(m_p_info.hThread);
    m_launched = false;
    return m_exit_code;
}
void Win32Process::setup_error_reader() {
    m_error_reader_thread = JThread{ [this](StopToken stoken) {
        if (!m_redirected_stderr) return;
        HANDLE hdl        = choose_handle(Win32PipeType::Error);
        while (!stoken.stop_requested()) { peek_and_read_pipe(hdl); }
    } };
}
void Win32Process::setup_output_reader() {
    m_output_reader_thread = JThread{ [this](StopToken stoken) {
        if (!m_redirected_stdout) return;
        HANDLE hdl         = choose_handle(Win32PipeType::Output);
        while (!stoken.stop_requested()) { peek_and_read_pipe(hdl); }
    } };
}
String Win32Process::get_last_error() {
    return ARLib::last_error();
}
ProcessResult Win32Process::launch() {
    LPVOID envp = NULL;
    if (m_environment.size() > 0) {
        envp = (m_dwflags & Win32ProcessFlags::UnicodeEnvironment) != Win32ProcessFlags::None ?
               reinterpret_cast<void*>(get<wchar_t*>(m_env_buffer)) :
               reinterpret_cast<void*>(get<char*>(m_env_buffer));
    }
    m_launched = CreateProcessA(
    NULL,    // lpApplicationName is NULL because otherwise it doesn't use the search path, the
             // application name is specified in the cmdline
    m_cmdline_buffer,
    NULL,    // lpProcessAttributes
    NULL,    // lpThreadAttributes
    TRUE,    // bInheritHandles
    static_cast<DWORD>(m_dwflags), envp, m_working_dir.has_value() ? m_working_dir.value().data() : NULL,
    cast<LPSTARTUPINFOA>(&m_s_info), cast<LPPROCESS_INFORMATION>(&m_p_info)
    );
    if (m_launched) m_exit_code = STILL_ACTIVE;
    setup_exit_handler();
    if (m_launch_waits_for_exit) {
        auto res = wait_for_exit();
        if (res.is_error()) { return res.to_error(); }
    }
    if (m_launched) {
        return {};
    } else {
        return get_last_error();
    }
}
bool Win32Process::good() const noexcept {
    return m_launched;
}
DWORD Win32Process::exit_code() const noexcept {
    return m_exit_code;
}
Win32Process::~Win32Process() {
    delete[] m_cmdline_buffer;
    if (m_env_buffer.is_active()) {
        if ((m_dwflags & Win32ProcessFlags::UnicodeEnvironment) != Win32ProcessFlags::None)
            delete[] get<wchar_t*>(m_env_buffer);
        else
            delete[] get<char*>(m_env_buffer);
    }
    for (auto* overlapped : m_overlapped) delete overlapped;
}
}    // namespace ARLib
#include "linux_native_process.h"

#include <cstdio>
#include <cstring>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
namespace ARLib {

MAKE_BITFIELD_ENUM(UnixProcessFlags)
int UnixProcess::choose_handle(UnixPipeType type) {
    int handles[3]{};
    bool redirects[3]{ m_redirected_stdout, m_redirected_stdin, m_redirected_stderr };
    if (redirects[from_enum(type)]) {
        handles[from_enum(UnixPipeType::Output)] = m_pipes.read_pipe(UnixPipeType::Output);
        handles[from_enum(UnixPipeType::Input)]  = m_pipes.write_pipe(UnixPipeType::Input);
        handles[from_enum(UnixPipeType::Error)]  = m_pipes.read_pipe(UnixPipeType::Error);
    } else {
        handles[from_enum(UnixPipeType::Output)] = fileno(stdout);
        handles[from_enum(UnixPipeType::Input)]  = fileno(stdin);
        handles[from_enum(UnixPipeType::Error)]  = fileno(stderr);
    }
    return handles[from_enum(type)];
}
UnixProcess::UnixProcess(UnixProcess&& proc) noexcept :
    m_pipes(proc.m_pipes), m_environment(move(proc.m_environment)), m_cmdline(move(proc.m_cmdline)),
    m_working_dir(move(proc.m_working_dir)), m_timeout(proc.m_timeout), m_exit_code(proc.m_exit_code),
    m_flags(proc.m_flags), m_envp(proc.m_envp), m_argv(proc.m_argv), m_child_pid(proc.m_child_pid),
    m_output(move(proc.m_output)), m_error(move(proc.m_error)),
    m_output_reader_thread(move(proc.m_output_reader_thread)), m_error_reader_thread(move(proc.m_error_reader_thread)),
    m_exit_handler_thread(move(proc.m_exit_handler_thread)), m_redirected_stdout(proc.m_redirected_stdout),
    m_redirected_stdin(proc.m_redirected_stdin), m_redirected_stderr(proc.m_redirected_stderr),
    on_output(move(proc.on_output)), on_error(move(proc.on_error)), on_exit(move(proc.on_exit)) {
    proc.m_launched  = false;
    proc.m_child_pid = -1;
    proc.m_envp      = nullptr;
    proc.m_argv      = nullptr;
}
UnixProcess& UnixProcess::operator=(UnixProcess&& proc) noexcept {
    m_pipes                = proc.m_pipes;
    m_environment          = move(proc.m_environment);
    m_cmdline              = move(proc.m_cmdline);
    m_working_dir          = move(proc.m_working_dir);
    m_timeout              = proc.m_timeout;
    m_exit_code            = proc.m_exit_code;
    m_flags                = proc.m_flags;
    m_envp                 = proc.m_envp;
    m_argv                 = proc.m_argv;
    m_child_pid            = proc.m_child_pid;
    m_output               = move(proc.m_output);
    m_error                = move(proc.m_error);
    m_output_reader_thread = move(proc.m_output_reader_thread);
    m_error_reader_thread  = move(proc.m_error_reader_thread);
    m_exit_handler_thread  = move(proc.m_exit_handler_thread);
    m_redirected_stdout    = proc.m_redirected_stdout;
    m_redirected_stdin     = proc.m_redirected_stdin;
    m_redirected_stderr    = proc.m_redirected_stderr;
    on_output              = move(proc.on_output);
    on_error               = move(proc.on_error);
    on_exit                = move(proc.on_exit);
    proc.m_launched        = false;
    proc.m_child_pid       = -1;
    proc.m_envp            = nullptr;
    proc.m_argv            = nullptr;
    return *this;
}
UnixProcess::UnixProcess(StringView command) : m_environment{} {
    m_cmdline = command.extract_string();
    m_argv    = new char*[2];
    m_argv[0] = new char[m_cmdline.size() + 1];
    m_argv[1] = nullptr;
    strcpy(m_argv[0], m_cmdline.data());
}
UnixProcess& UnixProcess::with_timeout(size_t ms_timeout) {
    m_timeout = ms_timeout;
    return *this;
}
UnixProcess& UnixProcess::with_cwd(StringView cwd) {
    m_working_dir = cwd.extract_string();
    return *this;
}
UnixProcess& UnixProcess::with_env(std::initializer_list<EnvironMapString> env_values) {
    for (const auto& env_value : env_values) { m_environment.add(env_value); }
    if (m_environment.size() > 0) {
        if (m_envp != nullptr) {
            for (size_t i = 0; m_envp[i]; i++) { delete[] m_envp[i]; }
            delete[] m_envp;
        }
        m_envp   = new char*[m_environment.size() + 1];
        size_t i = 0;
        for (const auto& [name, val] : m_environment) {
            m_envp[i] = new char[name.size() + 1 + val.size() + 1];
            memcpy(m_envp[i], name.string().data(), name.size());
            m_envp[i][name.size()] = '=';
            memcpy(m_envp[i] + name.size() + 1, val.string().data(), val.size());
            m_envp[i][name.size() + 1 + val.size()] = '\0';
            i++;
        }
        m_envp[i] = nullptr;
    }
    return *this;
}
UnixProcess& UnixProcess::with_args(std::initializer_list<ArgumentString> args) {
    if (args.size() > 0) {
        char** argv = new char*[args.size() + 2];
        argv[0]     = m_argv[0];    // copy over the program name argument
        size_t i    = 1;
        for (const auto& arg : args) {
            auto argument = arg.argument();
            char* arg_buf = new char[argument.size() + 1];
            strcpy(arg_buf, argument.data());
            argv[i] = arg_buf;
            i++;
        }
        argv[i] = nullptr;
        delete[] m_argv;
        m_argv = argv;
    }
    return *this;
}
UnixProcess& UnixProcess::with_flag(UnixProcessFlags flag) {
    m_flags = m_flags | flag;
    return *this;
}
UnixProcess& UnixProcess::set_blocking() {
    m_launch_waits_for_exit = true;
    return *this;
}
UnixProcess& UnixProcess::set_wait_at_destruction(bool) {
    m_wait_on_dtor = true;
    return *this;
}
String UnixProcess::get_last_error() {
    return last_error();
}
void UnixProcess::stop_and_join_thread(JThread& t) {
    if (t.joinable()) {
        t.request_stop();
        t.join();
    }
}
Result<exit_code_t, ProcessError> UnixProcess::wait_for_exit() {
    if (!m_launched || m_child_pid == -1)
        return Result<exit_code_t, ProcessError>::from_error("Process was not running"_s);
    if (m_timeout != INFINITE_TIMEOUT) {
        constexpr useconds_t tenms = 10 * 1000;
        const useconds_t usecs     = static_cast<useconds_t>(m_timeout) * 1000;
        useconds_t elapsed         = 0;
        while (m_process_exited.load() == false) {
            usleep(tenms);    // sleep for 10 ms, then recheck timeout
            elapsed += tenms;
            if (elapsed >= usecs) {
                m_exit_handler_thread.request_stop();
                return Result<exit_code_t, ProcessError>::from_error("Timeout reached waiting for process"_s);
            }
        }
    } else {
        if (m_exit_handler_thread.joinable()) m_exit_handler_thread.join();
    }
    stop_and_join_thread(m_output_reader_thread);
    stop_and_join_thread(m_error_reader_thread);
    return Result<exit_code_t, ProcessError>::from_ok(m_exit_code);
}
ProcessResult UnixProcess::wait_for_input(int) {
    return ProcessResult::from_error("Waiting for input is not supported on Linux"_s);
}
ProcessResult UnixProcess::suspend() {
    int ret = kill(m_child_pid, SIGSTOP);
    if (ret == 0) return ProcessResult::from_ok();
    return ProcessResult::from_error(last_error());
}
ProcessResult UnixProcess::resume() {
    int ret = kill(m_child_pid, SIGCONT);
    if (ret == 0) return ProcessResult::from_ok();
    return ProcessResult::from_error(last_error());
}
bool UnixProcess::terminate(exit_code_t exit_code) {
    (void)exit_code;    // unusued for now
    int ret = kill(m_child_pid, SIGKILL);
    return ret == 0;
}
bool UnixProcess::good() const noexcept {
    return m_launched && m_child_pid != -1;
}
exit_code_t UnixProcess::exit_code() const noexcept {
    return m_exit_code;
}
void UnixProcess::setup_output_reader() {
    m_output_reader_thread = JThread{ [this](StopToken stoken) {
        if (!m_redirected_stdout) return;
        int fd             = choose_handle(UnixPipeType::Output);
        while (!stoken.stop_requested()) { peek_and_read_pipe(fd); }
    } };
}
void UnixProcess::setup_error_reader() {
    m_error_reader_thread = JThread{ [this](StopToken stoken) {
        if (!m_redirected_stderr) return;
        int fd            = choose_handle(UnixPipeType::Error);
        while (!stoken.stop_requested()) { peek_and_read_pipe(fd); }
    } };
}
void UnixProcess::setup_exit_handler() {
    m_exit_handler_thread = JThread{ [this](StopToken stoken) {
        siginfo_t info{};
        // if I sleep in the child process apparently this returns -1 ??????
        auto ret          = waitid(P_PID, static_cast<id_t>(m_child_pid), &info, WEXITED);
        if (ret == -1) print_last_error();
        if (!stoken.stop_requested()) {
            m_exit_code = info.si_status;
            on_exit(info.si_status);
            // unregister the callback
            on_exit.unregister();
        }
        bool expected     = false;
        m_process_exited.compare_exchange_strong(expected, true);
    } };
}
void UnixProcess::peek_and_read_pipe(int pipe) {
    size_t bytesAvailable = 0;
    auto err              = ioctl(pipe, FIONREAD, &bytesAvailable);
    if (bytesAvailable == 0) return;
    if (err == -1) return;
    CharPtr buffer{ bytesAvailable + 1 };
    auto read_bytes = static_cast<size_t>(read(pipe, buffer.ptr, bytesAvailable));
    if (read_bytes != bytesAvailable) return;
    buffer.ptr[bytesAvailable] = '\0';
    if (pipe == m_pipes.read_pipe(UnixPipeType::Output)) {
        auto bytes = ReadOnlyView<uint8_t>{ reinterpret_cast<uint8_t*>(buffer.ptr), static_cast<size_t>(read_bytes) };
        on_output(OutputType{ bytes });
        m_output.append(bytes);
    } else {
        auto bytes = ReadOnlyView<uint8_t>{ reinterpret_cast<uint8_t*>(buffer.ptr), static_cast<size_t>(read_bytes) };
        on_error(OutputType{ bytes });
        m_error.append(bytes);
    }
}
ProcessResult UnixProcess::write_input(StringView input, UnixProcess::completion_routine_t routine) {
    if (!m_redirected_stdin)
        return ProcessResult::from_error("To write to a process' input without pipes just use stdin"_s);
    int fd = choose_handle(UnixPipeType::Input);
    if (routine == nullptr) {
        if (static_cast<size_t>(write(fd, input.data(), input.size())) != input.size())
            return ProcessResult::from_error(last_error());
    } else {
        JThread writer_thread{ [=]() {
            auto written = write(fd, input.data(), input.size());
            routine(errno, static_cast<size_t>(written), nullptr);
        } };
        writer_thread.detach();
    }
    return ProcessResult::from_ok();
}
ProcessResult UnixProcess::write_data(const ReadOnlyView<uint8_t>& data, UnixProcess::completion_routine_t routine) {
    if (!m_redirected_stdin)
        return ProcessResult::from_error("To write to a process' input without pipes just use stdin"_s);
    int fd = choose_handle(UnixPipeType::Input);
    if (routine == nullptr) {
        if (static_cast<size_t>(write(fd, data.data(), data.size())) != data.size())
            return ProcessResult::from_error(last_error());
    } else {
        JThread writer_thread{ [=]() {
            auto written = write(fd, data.data(), data.size());
            routine(errno, static_cast<size_t>(written), nullptr);
        } };
        writer_thread.detach();
    }
    return ProcessResult::from_ok();
}
ProcessResult UnixProcess::set_pipe(UnixPipeType type) {
    int ret_val = 0;
    using rbool = RefBox<bool>;
    rbool redirects[3]{ m_redirected_stdout, m_redirected_stdin, m_redirected_stderr };
    redirects[from_enum(type)].get() = true;
    switch (type) {
        case UnixPipeType::Output:
            ret_val = pipe(m_pipes.output);
            setup_output_reader();
            break;
        case UnixPipeType::Input:
            ret_val = pipe(m_pipes.input);
            break;
        case UnixPipeType::Error:
            ret_val = pipe(m_pipes.error);
            setup_error_reader();
            break;
    }
    if (ret_val == -1) {
        redirects[from_enum(type)].get() = false;
        return ProcessResult::from_error("Cannot open pipe"_s);
    }
    return ProcessResult::from_ok();
}
ProcessResult UnixProcess::flush_input() {
    auto fd = choose_handle(UnixPipeType::Input);
    int ret = fsync(fd);
    if (ret != 0) return ProcessResult::from_error("Couldn't flush pipe"_s);
    return ProcessResult::from_ok();
}
ProcessResult UnixProcess::close_pipe(UnixPipeType type) {
    int ret_val = 0;
    bool redirects[3]{ m_redirected_stdout, m_redirected_stdin, m_redirected_stderr };
    if (!redirects[from_enum(type)]) { return ProcessResult::from_error("Pipe is already closed"_s); }
    switch (type) {
        case UnixPipeType::Input:
            ret_val = close(m_pipes.write_pipe(type));
            break;
        case UnixPipeType::Output:
            stop_and_join_thread(m_output_reader_thread);
            ret_val = close(m_pipes.read_pipe(type));
            break;
        case UnixPipeType::Error:
            stop_and_join_thread(m_error_reader_thread);
            ret_val = close(m_pipes.read_pipe(type));
            break;
    }
    if (ret_val == -1) { return ProcessResult::from_error("Cannot close pipe"_s); }
    return ProcessResult::from_ok();
}
char** copy_environ() {
    size_t count         = 0;
    char** const og_envp = environ;
    for (const char* envp = *og_envp; envp; envp = og_envp[++count])
        ;
    char** copied_env = new char*[count + 1];
    for (size_t i = 0; i < count; i++) {
        size_t s_len  = strlen(og_envp[i]);
        copied_env[i] = new char[s_len + 1];
        strcpy(copied_env[i], og_envp[i]);
    }
    copied_env[count] = nullptr;
    return copied_env;
}
ProcessResult UnixProcess::launch() {
    if (m_environment.size() == 0) { m_envp = copy_environ(); }
    pid_t pid = fork();
    switch (pid) {
        case 0:
            // new process here
            if (m_redirected_stdout) { dup2(m_pipes.write_pipe(UnixPipeType::Output), STDOUT_FILENO); }
            if (m_redirected_stdin) { dup2(m_pipes.read_pipe(UnixPipeType::Input), STDIN_FILENO); }
            if (m_redirected_stderr) { dup2(m_pipes.write_pipe(UnixPipeType::Output), STDERR_FILENO); }
            m_pipes.close(m_redirected_stdout, m_redirected_stdin, m_redirected_stderr);
            if (m_working_dir.has_value()) chdir(m_working_dir.value().data());
            execvpe(m_cmdline.data(), m_argv, m_envp);
            // if success, it doesn't go here, so if we continue we know for sure execve failed
            [[fallthrough]];
        case -1:
            return ProcessResult::from_error(last_error());
        default:
            // old process
            m_launched  = true;
            m_child_pid = pid;
            setup_exit_handler();
    }
    if (m_launch_waits_for_exit) {
        if (auto res = wait_for_exit(); res.is_error()) { return ProcessResult::from_error(res.to_error()); };
    }
    return ProcessResult::from_ok();
}
const OutputType& UnixProcess::output() const {
    return m_output;
}
const OutputType& UnixProcess::error() const {
    return m_error;
}
UnixProcess::~UnixProcess() {
    if (m_envp != nullptr) {
        for (size_t i = 0; m_envp[i]; i++) { delete[] m_envp[i]; }
        delete[] m_envp;
    }
    if (m_argv != nullptr) {
        for (size_t i = 0; m_argv[i]; i++) { delete[] m_argv[i]; }
        delete[] m_argv;
    }
    if (m_launched && m_wait_on_dtor && m_child_pid == -1) { wait_for_exit(); }
}
}    // namespace ARLib
#pragma once
#include "Compat.hpp"
#include "Functional.hpp"
#include "Threading.hpp"
#include "ImplProcessCommon.hpp"
#include "Result.hpp"
#include "Atomic.hpp"
#include "FlatMap.hpp"
#include "linux_native_io.hpp"

#ifdef UNIX
namespace ARLib {
class UnixProcess;
template <typename... Arguments>
class UnixCallback {
    friend UnixProcess;
    using callback_t = Function<void(Arguments...)>;
    callback_t callback;
    UnixProcess* owner_process;
    Mutex m;
    UnixCallback(UnixProcess* process) : owner_process(process) {}
    UnixCallback()                    = delete;
    UnixCallback(const UnixCallback&) = delete;
    UnixCallback(UnixCallback&& cb) noexcept : callback(move(cb.callback)), owner_process(cb.owner_process) {
        cb.owner_process = nullptr;
    }
    UnixCallback& operator=(const UnixCallback&) = delete;
    UnixCallback& operator=(UnixCallback&& cb) noexcept {
        callback         = move(cb.callback);
        owner_process    = cb.owner_process;
        cb.owner_process = nullptr;
        return *this;
    };

    public:
    UnixCallback& operator+=(callback_t&& fn) {
        ScopedLock l{ m };
        callback = fn;
        return *this;
    }
    UnixCallback& operator-=(const callback_t&) {
        ScopedLock l{ m };
        callback = callback_t{};
        return *this;
    }
    void unregister() {
        ScopedLock l{ m };
        callback = callback_t{};
    }
    void operator()(Arguments... args) {
        bool exists = false;
        {
            ScopedLock l{ m };
            exists = bool{ callback };
        }
        if (exists) callback(Forward<Arguments>(args)...);
    }
};

enum class UnixProcessFlags : int {
    None                       = 0x00000000,
    BreakawayFromJob           = 0x00000000,
    DefaultErrorMode           = 0x00000000,
    NewConsole                 = 0x00000000,
    NewProcessGroup            = 0x00000000,
    NoWindow                   = 0x00000000,
    PreserveCodeAuthzLevel     = 0x00000000,
    SecureProcess              = 0x00000000,
    SeparateWowVdm             = 0x00000000,
    SharedWowVdm               = 0x00000000,
    Suspended                  = 0x00000000,
    UnicodeEnvironment         = 0x00000000,
    DebugOnlyThisProcess       = 0x00000000,
    DebugProcess               = 0x00000000,
    DetachedProcess            = 0x00000000,
    ExtendedStartupInfoPresent = 0x00000000,
    InheritParentAffinity      = 0x00000000
};

enum class UnixPipeType { Output = 0, Input = 1, Error = 2 };

using exit_code_t = int;
using timeout_t   = size_t;
using handle_type = UnixPipeType;
    #define EXIT_FAILURE 1

constexpr auto INFINITE_TIMEOUT = static_cast<size_t>(-1);
class UnixProcess {
    struct {
        int output[2];
        int input[2];
        int error[2];
        inline void close(bool out, bool in, bool err) {
            if (out) UnixClose(read_pipe(UnixPipeType::Output));
            if (in) UnixClose(write_pipe(UnixPipeType::Input));
            if (err) UnixClose(read_pipe(UnixPipeType::Error));
        }
        inline int read_pipe(UnixPipeType type) {
            switch (type) {
                case UnixPipeType::Output:
                    return output[0];
                case UnixPipeType::Input:
                    return input[0];
                case UnixPipeType::Error:
                    return error[0];
            }
            return -1;
        }
        inline int write_pipe(UnixPipeType type) {
            switch (type) {
                case UnixPipeType::Output:
                    return output[1];
                case UnixPipeType::Input:
                    return input[1];
                case UnixPipeType::Error:
                    return error[1];
            }
            return -1;
        }
    } m_pipes;
    FlatMap<EnvironString, EnvironString> m_environment;

    Atomic<bool> m_process_exited = false;
    String m_cmdline;
    Optional<String> m_working_dir;
    timeout_t m_timeout      = INFINITE_TIMEOUT;
    exit_code_t m_exit_code  = EXIT_FAILURE;
    UnixProcessFlags m_flags = UnixProcessFlags::None;
    char** m_envp            = nullptr;
    char** m_argv            = nullptr;
    int m_child_pid          = -1;

    OutputType m_output;
    OutputType m_error;

    JThread m_output_reader_thread{};
    JThread m_error_reader_thread{};
    JThread m_exit_handler_thread{};

    Atomic<bool> m_redirected_stdout = false;
    Atomic<bool> m_redirected_stdin  = false;
    Atomic<bool> m_redirected_stderr = false;

    void setup_output_reader();
    void setup_error_reader();
    void setup_exit_handler();

    void peek_and_read_pipe(int pipe);

    int choose_handle(UnixPipeType);

    bool m_wait_on_dtor          = true;
    bool m_launched              = false;
    bool m_launch_waits_for_exit = false;

    void stop_and_join_thread(JThread& t);

    public:
    using EnvironMapString          = decltype(m_environment)::ValueType;
    UnixProcess()                   = default;
    UnixProcess(const UnixProcess&) = delete;
    UnixProcess(UnixProcess&&) noexcept;
    UnixProcess& operator=(const UnixProcess&) = delete;
    UnixProcess& operator=(UnixProcess&&) noexcept;

    UnixProcess(StringView command);
    UnixProcess& with_timeout(size_t ms_timeout);
    UnixProcess& with_args(std::initializer_list<ArgumentString>);
    UnixProcess& with_cwd(StringView cwd);
    UnixProcess& with_env(std::initializer_list<EnvironMapString>);
    UnixProcess& with_flag(UnixProcessFlags flag);

    UnixProcess& set_blocking();
    UnixProcess& set_wait_at_destruction(bool wait = true);
    String get_last_error();
    Result<exit_code_t, Error> wait_for_exit();
    ProcessResult wait_for_input(int ms_timeout = -1);
    ProcessResult launch();
    ProcessResult suspend();
    ProcessResult resume();
    bool terminate(exit_code_t exit_code = 0);
    bool good() const noexcept;
    exit_code_t exit_code() const noexcept;

    ProcessResult set_pipe(UnixPipeType);
    ProcessResult close_pipe(UnixPipeType);
    ProcessResult flush_input();

    UnixCallback<OutputType> on_output{ this };
    UnixCallback<OutputType> on_error{ this };
    UnixCallback<exit_code_t> on_exit{ this };

    const OutputType& output() const;
    const OutputType& error() const;

    using completion_routine_t = void (*)(int errc, size_t written, void* up);

    ProcessResult write_input(StringView input, completion_routine_t routine = nullptr);
    ProcessResult write_data(const ReadOnlyView<uint8_t>& data, completion_routine_t routine = nullptr);

    ~UnixProcess();
};
}    // namespace ARLib
#endif
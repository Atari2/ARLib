#pragma once
#include "../Compat.h"
#include "../Functional.h"
#include "../Threading.h"
#include "../FlatMap.h"
#include "../ImplProcessCommon.h"
#include "../Result.h"

#ifdef WINDOWS
namespace ARLib {
class Win32Process;
template <typename... Arguments>
class Win32Callback {
    friend Win32Process;
    using callback_t = Function<void(Arguments...)>;
    callback_t callback;
    Win32Process* owner_process;
    Mutex m;
    Win32Callback(Win32Process* process) : owner_process(process) {}
    Win32Callback()                     = delete;
    Win32Callback(const Win32Callback&) = delete;
    Win32Callback(Win32Callback&& cb) noexcept : callback(move(cb.callback)), owner_process(cb.owner_process) {
        cb.owner_process = nullptr;
    }
    Win32Callback& operator=(const Win32Callback&) = delete;
    Win32Callback& operator=(Win32Callback&& cb) noexcept {
        owner_process    = cb.owner_process;
        callback         = move(cb.callback);
        cb.owner_process = nullptr;
        return *this;
    }

    public:
    Win32Callback& operator+=(callback_t&& fn) {
        ScopedLock l{ m };
        callback = fn;
        return *this;
    }
    Win32Callback& operator-=(const callback_t&) {
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

enum class Win32ProcessFlags : WinDword {
    None                       = 0x00000000,
    BreakawayFromJob           = 0x01000000,
    DefaultErrorMode           = 0x04000000,
    NewConsole                 = 0x00000010,
    NewProcessGroup            = 0x00000200,
    NoWindow                   = 0x08000000,
    PreserveCodeAuthzLevel     = 0x02000000,
    SecureProcess              = 0x00400000,
    SeparateWowVdm             = 0x00000800,
    SharedWowVdm               = 0x00001000,
    Suspended                  = 0x00000004,
    UnicodeEnvironment         = 0x00000400,
    DebugOnlyThisProcess       = 0x00000002,
    DebugProcess               = 0x00000001,
    DetachedProcess            = 0x00000008,
    ExtendedStartupInfoPresent = 0x00080000,
    InheritParentAffinity      = 0x00010000
};

enum class Win32PipeType { Output = 0, Input = 1, Error = 2 };

using exit_code_t = WinDword;
using timeout_t   = WinDword;
using handle_type = Win32PipeType;
class Win32Process {
    struct Pipes {
        WinHandle output = ARLIB_INVALID_HANDLE_VALUE;
        WinHandle input  = ARLIB_INVALID_HANDLE_VALUE;
        WinHandle error  = ARLIB_INVALID_HANDLE_VALUE;
    } m_pipes;
    OutputType m_output{};
    OutputType m_error{};
    Optional<String> m_working_dir;

    FlatMap<EnvironString, EnvironString> m_environment;
    Vector<WinHandle> m_internal_handles{};
    Vector<internal::LPOverlapped> m_overlapped{};
    internal::ProcessInformation m_p_info{};
    internal::StartupInfoA m_s_info{};
    String m_proc_name{};
    char* m_cmdline_buffer = nullptr;
    Variant<char*, wchar_t*> m_env_buffer;
    bool m_wait_on_dtor          = true;
    bool m_launched              = false;
    bool m_launch_waits_for_exit = false;
    timeout_t m_timeout          = ARLIB_INFINITE_TIMEOUT;
    exit_code_t m_exit_code      = EXIT_FAILURE;
    Win32ProcessFlags m_dwflags  = Win32ProcessFlags::None;
    bool m_redirected_stdin      = false;
    bool m_redirected_stdout     = false;
    bool m_redirected_stderr     = false;
    JThread m_output_reader_thread{};
    JThread m_error_reader_thread{};
    JThread m_exit_handler_thread{};

    void setup_output_reader();
    void setup_error_reader();
    void setup_exit_handler();

    void peek_and_read_pipe(WinHandle pipe);

    WinHandle choose_handle(Win32PipeType);
    void stop_and_join_thread(JThread& t);

    public:
    using EnvironMapString            = decltype(m_environment)::ValueType;
    Win32Process()                    = default;
    Win32Process(const Win32Process&) = delete;
    Win32Process(Win32Process&&) noexcept;
    Win32Process& operator=(const Win32Process&) = delete;
    Win32Process& operator=(Win32Process&&) noexcept;

    Win32Process(StringView command);
    Win32Process& with_timeout(size_t ms_timeout);
    Win32Process& with_args(std::initializer_list<ArgumentString> args);
    Win32Process& with_cwd(StringView cwd);
    Win32Process& with_env(std::initializer_list<EnvironMapString>);
    Win32Process& with_flag(Win32ProcessFlags flag);
    Win32Process& set_blocking();
    Win32Process& set_wait_at_destruction(bool wait = true);
    String get_last_error();
    Result<exit_code_t> wait_for_exit();
    ProcessResult wait_for_input(size_t ms_timeout = ARLIB_INFINITE_TIMEOUT);
    ProcessResult launch();
    ProcessResult suspend();
    ProcessResult resume();
    bool terminate(exit_code_t exit_code = 0);
    bool good() const noexcept;
    exit_code_t exit_code() const noexcept;

    ProcessResult set_pipe(Win32PipeType);
    ProcessResult close_pipe(Win32PipeType);
    ProcessResult flush_input();

    Win32Callback<OutputType> on_output{ this };
    Win32Callback<OutputType> on_error{ this };
    Win32Callback<WinDword> on_exit{ this };

    const OutputType& output() const;
    const OutputType& error() const;

    using completion_routine_t = internal::LPOverlappedCompletionRoutine;

    ProcessResult write_input(StringView input, completion_routine_t routine = nullptr);
    ProcessResult write_data(const ReadOnlyView<uint8_t>& data, completion_routine_t routine = nullptr);
    ~Win32Process();
};
    #define WIN32_IO_COMPLETION_ROUTINE(func_name, arg0, arg1, arg2)                                                   \
        void __stdcall func_name(WinDword arg0, WinDword arg1, internal::LPOverlapped arg2)
}    // namespace ARLib
#endif

#pragma once
#include "Compat.h"
#ifdef WINDOWS
    #include "Windows/win_native_process.h"
#else
    #include "Linux/linux_native_process.h"
#endif
namespace ARLib {
#ifdef WINDOWS
using ProcessFlags = Win32ProcessFlags;
using Process      = Win32Process;
using HandleType   = Win32PipeType;
#else
using ProcessFlags = UnixProcessFlags;
using Process      = UnixProcess;
using HandleType   = UnixPipeType;
#endif
class ProcessPipeline {
    Vector<Process> m_processes;

    public:
    ProcessPipeline(Process&& lhs, Process&& rhs);
    ProcessPipeline(ProcessPipeline&& pipe, Process&& lhs);

    DiscardResult<ProcessError> run();
    inline auto begin() const { return m_processes.begin(); }
    inline auto end() const { return m_processes.end(); }
    inline size_t size() const { return m_processes.size(); }
    inline const OutputType& output() const { return m_processes.last().output(); }
    inline const OutputType& output(size_t idx) const { return m_processes[idx].output(); }
};
ProcessPipeline operator|(ProcessPipeline&& pipeline, Process&& lhs);
ProcessPipeline operator|(Process& lhs, Process& rhs);
ProcessPipeline operator|(Process&& lhs, Process&& rhs);
ProcessPipeline operator|(Process& lhs, Process&& rhs);
ProcessPipeline operator|(Process&& lhs, Process& rhs);
}    // namespace ARLib
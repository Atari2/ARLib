#include "Process.hpp"
namespace ARLib {
ProcessPipeline::ProcessPipeline(Process&& lhs, Process&& rhs) {
    m_processes.append(Forward<Process>(lhs));
    m_processes.append(Forward<Process>(rhs));
}
ProcessPipeline::ProcessPipeline(ProcessPipeline&& pipe, Process&& lhs) : m_processes(move(pipe.m_processes)) {
    m_processes.push_back(Forward<Process>(lhs));
}
Result<exit_code_t> ProcessPipeline::run() {
    String last_output;
    exit_code_t last_exit_code{};
    for (size_t i = 0; i < m_processes.size(); i++) {
        auto& proc = m_processes[i];
        TRY(proc.set_pipe(HandleType::Output));
        TRY(proc.set_pipe(HandleType::Input));
        TRY(proc.launch());
        if (last_output.size() > 0) {
            TRY(proc.write_input(last_output.view()));
            TRY(proc.flush_input());
            TRY(proc.close_pipe(HandleType::Input));
        }
        TRY_SET(ec, proc.wait_for_exit());
        last_exit_code = ec;
        last_output    = proc.output();
    }
    return last_exit_code;
}
ProcessPipeline operator|(ProcessPipeline&& pipeline, Process&& lhs) {
    return ProcessPipeline{ Forward<ProcessPipeline>(pipeline), Forward<Process>(lhs) };
}
ProcessPipeline operator|(Process& lhs, Process& rhs) {
    return ProcessPipeline{ move(lhs), move(rhs) };
}
ProcessPipeline operator|(Process&& lhs, Process&& rhs) {
    return ProcessPipeline{ Forward<Process>(lhs), Forward<Process>(rhs) };
}
ProcessPipeline operator|(Process& lhs, Process&& rhs) {
    return ProcessPipeline{ move(lhs), Forward<Process>(rhs) };
}
ProcessPipeline operator|(Process&& lhs, Process& rhs) {
    return ProcessPipeline{ Forward<Process>(lhs), move(rhs) };
}
}    // namespace ARLib
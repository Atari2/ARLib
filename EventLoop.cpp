#ifndef DISABLE_THREADING
    #include "EventLoop.h"
namespace ARLib {
void EventLoop::loop_function(EventLoop* loop) {
    while (loop->running()) {
        if (loop->m_callbacks.size() > 0) {
            Function<void()> callback;
            {
                UniqueLock lock{ loop->m_callback_loc };
                callback = move(loop->m_callbacks.pop());
                if (loop->m_callbacks.size() == 0) loop->stop();
            }
            callback();
        }
    }
}
void EventLoop::start() {
    m_running = true;
    m_thread  = Thread{ loop_function, this };
}
}    // namespace ARLib
#endif

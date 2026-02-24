#include "Logger.hpp"
namespace ARLib {
void ARLib::LoggingBackendTs::log(LogLevel level, StringView message) {
    ScopedLock lock{ m_mutex };
    this->_log_ts(level, message);
}
void ConsoleLoggerTs::_log_ts(LogLevel level, StringView message) {
    if (!should_log(level)) return;
    Printer::print("{}", message);
}
void ARLib::ConsoleLogger::log(LogLevel level, StringView message) {
    if (!should_log(level)) return;
    Printer::print("{}", message);
}
void Logger::LoggingStorage::add_backend(const String& name, SharedPtr<LoggingBackend> backend) {
    m_store.with_lock([&name, backend = move(backend)](LoggingStore& map) { 
        map.insert(name, move(backend));
    });
}
void Logger::LoggingStorage::remove_backend(const String& name) {
    m_store.with_lock([&name](LoggingStore& map) { 
        map.remove(name); 
    });
}
Logger::LoggingStorage::ResultType Logger::LoggingStorage::get(StringView name) {
    auto store = m_store.lock();
    if (auto it = store->find(name); it != store->end()) {
        return (*it).val();
    } else {
        return LoggerStorageError::NotFound;
    }
}
Logger::LoggingStorage::ResultType Logger::get_named_logger(StringView name) {
    return store().get(name);
}
Logger::LoggingStorage::ResultType Logger::get_default_logger() {
    return store().get("default"_sv);
}

}    // namespace ARLib
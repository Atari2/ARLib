#include "Logger.hpp"
namespace ARLib {
void Logger::LoggingStorage::add_backend(const String& name, SharedPtr<LoggingBackend> backend) {
    ScopedLock lock(m_mutex);
    m_store.insert(name, move(backend));
}
void Logger::LoggingStorage::remove_backend(const String& name) {
    ScopedLock lock(m_mutex);
    m_store.remove(name);
}
Logger::LoggingStorage::ResultType Logger::LoggingStorage::get(StringView name) {
    if (auto it = m_store.find(name); it != m_store.end()) {
        return (*it).val();
    } else {
        return LoggerStorageError::NotFound;
    }
}
Logger::LoggingStorage::ResultTypeTs Logger::LoggingStorage::get_ts(StringView name) {
    UniqueLock lock{ m_mutex };
    if (auto it = m_store.find(name); it != m_store.end()) {
        auto backend = (*it).val();
        return Pair{ move(backend), move(lock) };
    } else {
        return LoggerStorageError::NotFound;
    }
}
void Logger::LoggingStorage::free(ResultTypeTs locked_backend) {
    if (locked_backend.is_error()) {
        locked_backend.ignore_error();
    }

}
void Logger::LoggingStorage::free(Pair<SharedPtr<LoggingBackend>, UniqueLock<Mutex>> locked_backend) {
    // the destructor will do the work.
}
void Logger::register_default_logger(SharedPtr<LoggingBackend> backend) {
    store().add_backend("default"_s, move(backend));
}
void Logger::register_named_logger(String name, SharedPtr<LoggingBackend> backend) {
    store().add_backend(name, move(backend));
}

}    // namespace ARLib
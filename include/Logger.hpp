#pragma once
#include "FlatMap.hpp"
#include "SharedPtr.hpp"
#include "Threading.hpp"
#include "Result.hpp"
#include "Stream.hpp"
namespace ARLib {

MAKE_FANCY_ENUM(LogLevel, uint8_t, Error = 0, Warning = 1, Info = 2, Debug = 3, Trace = 4);
class LoggingBackend {
    LogLevel m_level;

    public:
    LoggingBackend(LogLevel level) : m_level{ level } {} 
    constexpr bool should_log(LogLevel level) const { return level <= m_level; }
};

class ConsoleLogger : public LoggingBackend {
    public:
    ConsoleLogger(LogLevel level) : LoggingBackend{ level } {}
};


template <DerivedFrom<BaseStream> T>
class StreamLogger : public LoggingBackend {
    T m_stream;
    public:
    StreamLogger(LogLevel level, T stream) : LoggingBackend{ level }, m_stream{ move(stream) } {}
};

class FileLogger : public StreamLogger<FileStream> {
    String m_file_path;
    public:
    FileLogger(LogLevel level, String file_path) :
        StreamLogger<FileStream>{ level, FileStream{file_path} }, m_file_path{ move(file_path) } {}
};

class BufferedFileLogger : public StreamLogger<BufferedFileStream> {
    String m_file_path;
    public:
    BufferedFileLogger(LogLevel level, String file_path) :
        StreamLogger<BufferedFileStream>{ level, BufferedFileStream{ file_path } }, m_file_path{ move(file_path) } {}
};

class StringLogger : public StreamLogger<StringStream> {
    public:
    StringLogger(LogLevel level) : StreamLogger<StringStream>{ level, StringStream{} } {}
};


using LoggingStore = FlatMap<String, SharedPtr<LoggingBackend>>;

MAKE_FANCY_ENUM(LoggerStorageError, uint8_t, NotFound);
class Logger {
    class LoggingStorage {
        friend Logger;
        LoggingStore m_store{};
        Mutex m_mutex{};
        using ResultType   = Result<SharedPtr<LoggingBackend>, LoggerStorageError>;
        using ResultTypeTs = Result<Pair<SharedPtr<LoggingBackend>, UniqueLock<Mutex>>, LoggerStorageError>;
        LoggingStorage()                                 = default;
        LoggingStorage(const LoggingStorage&)            = delete;
        LoggingStorage(LoggingStorage&&)                 = delete;
        LoggingStorage& operator=(const LoggingStorage&) = delete;
        LoggingStorage& operator=(LoggingStorage&&)      = delete;

        void add_backend(const String& name, SharedPtr<LoggingBackend> backend);
        void remove_backend(const String& name);
        ResultType get(StringView name);
        ResultTypeTs get_ts(StringView name);
        void free(ResultTypeTs locked_backend);
        void free(Pair<SharedPtr<LoggingBackend>, UniqueLock<Mutex>> locked_backend);
    };
    static LoggingStorage& store() {
        static LoggingStorage store{};
        return store;
    }
    public:
    static void register_default_logger(SharedPtr<LoggingBackend> backend);
    static void register_named_logger(String name, SharedPtr<LoggingBackend> backend);
};
}    // namespace ARLib
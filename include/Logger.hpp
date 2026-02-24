#pragma once
#include "FlatMap.hpp"
#include "SharedPtr.hpp"
#include "Threading.hpp"
#include "Result.hpp"
#include "Stream.hpp"
#include "FormatString.hpp"
#include "Sync.hpp"
namespace ARLib {
constexpr static inline uint8_t _newline_buffer[]{ '\n' };
constexpr static inline Span<const uint8_t> _newline_span{ _newline_buffer };
MAKE_FANCY_ENUM(LogLevel, uint8_t, Critical = 5, Error = 4, Warning = 3, Info = 2, Debug = 1, Trace = 0);
class LoggingBackend {
    LogLevel m_level;
    friend class LoggingBackendTs;
    public:
    LoggingBackend(LogLevel level) : m_level{ level } {}
    constexpr bool should_log(LogLevel level) const { return level >= m_level; }
    virtual void log(LogLevel level, StringView message) = 0;
    virtual ~LoggingBackend()                            = default;
};
class LoggingBackendTs : public LoggingBackend {
    Mutex m_mutex;

    virtual void _log_ts(LogLevel level, StringView message) = 0;

    public:
    LoggingBackendTs(LogLevel level) : LoggingBackend{ level }, m_mutex{} {}
    constexpr bool should_log(LogLevel level) const { return level >= m_level; }
    void log(LogLevel level, StringView message);
    virtual ~LoggingBackendTs() = default;
};
class ConsoleLogger : public LoggingBackend {
    public:
    ConsoleLogger(LogLevel level) : LoggingBackend{ level } {}
    void log(LogLevel level, StringView message) override;
};
class ConsoleLoggerTs : public LoggingBackendTs {
    void _log_ts(LogLevel level, StringView message);
    public:
    ConsoleLoggerTs(LogLevel level) : LoggingBackendTs{ level } {}
};
template <DerivedFrom<BaseStream> T>
class StreamLogger : public LoggingBackend {
    UniquePtr<T> m_stream;
    friend class FileLogger;
    friend class BufferedFileLogger;
    friend class StringLogger;
    public:
    StreamLogger(LogLevel level, T stream) : LoggingBackend{ level }, m_stream{ move(stream) } {}
    void log(LogLevel level, StringView message) override {
        if (!should_log(level)) return;
        Span<const uint8_t> bytes{ reinterpret_cast<const uint8_t*>(message.data()), message.size() };
        auto result = m_stream->write(bytes);
        m_stream->write(_newline_span);
        HARD_ASSERT(result.is_ok(), "Failed to write to log stream");
    }
};
template <DerivedFrom<BaseStream> T>
class StreamLoggerTs : public LoggingBackendTs {
    UniquePtr<T> m_stream;
    friend class FileLoggerTs;
    friend class BufferedFileLoggerTs;
    friend class StringLoggerTs;
    void _log_ts(LogLevel level, StringView message) override {
        if (!should_log(level)) return;
        Span<const uint8_t> bytes{ reinterpret_cast<const uint8_t*>(message.data()), message.size() };
        auto result = m_stream->write(bytes);
        m_stream->write(_newline_span);
        HARD_ASSERT(result.is_ok(), "Failed to write to log stream");
    }
    public:
    StreamLoggerTs(LogLevel level, T stream) : LoggingBackendTs{ level }, m_stream{ move(stream) } {}
};
class FileLogger : public StreamLogger<FileStream> {
    String m_file_path;
    public:
    FileLogger(LogLevel level, String file_path) :
        StreamLogger<FileStream>{ level, FileStream{ file_path } }, m_file_path{ move(file_path) } {
        auto res = m_stream->open();
        HARD_ASSERT(res.is_ok(), "Failed to open log file");
    }
};
class BufferedFileLogger : public StreamLogger<BufferedFileStream> {
    String m_file_path;
    public:
    BufferedFileLogger(LogLevel level, String file_path) :
        StreamLogger<BufferedFileStream>{ level, BufferedFileStream{ file_path } }, m_file_path{ move(file_path) } {
        auto res = m_stream->open();
        HARD_ASSERT(res.is_ok(), "Failed to open log file");
    }
};
class StringLogger : public StreamLogger<StringStream> {
    public:
    StringLogger(LogLevel level) : StreamLogger<StringStream>{ level, StringStream{} } {}
    String output() const { return m_stream->str(); }
};
class FileLoggerTs : public StreamLoggerTs<FileStream> {
    String m_file_path;
    public:
    FileLoggerTs(LogLevel level, String file_path) :
        StreamLoggerTs<FileStream>{ level, FileStream{ file_path } }, m_file_path{ move(file_path) } {
        auto res = m_stream->open();
        HARD_ASSERT(res.is_ok(), "Failed to open log file");
    }
};
class BufferedFileLoggerTs : public StreamLoggerTs<BufferedFileStream> {
    String m_file_path;
    public:
    BufferedFileLoggerTs(LogLevel level, String file_path) :
        StreamLoggerTs<BufferedFileStream>{ level, BufferedFileStream{ file_path } }, m_file_path{ move(file_path) } {
        auto res = m_stream->open();
        HARD_ASSERT(res.is_ok(), "Failed to open log file");
    }
};
class StringLoggerTs : public StreamLoggerTs<StringStream> {
    public:
    StringLoggerTs(LogLevel level) : StreamLoggerTs<StringStream>{ level, StringStream{} } {}
    String output() const { return m_stream->str(); }
};
using LoggingStore = FlatMap<String, SharedPtr<LoggingBackend>>;

MAKE_FANCY_ENUM(LoggerStorageError, uint8_t, NotFound);
class Logger {
    class LoggingStorage {
        friend Logger;
        SyncData<LoggingStore> m_store{ {} };
        using ResultType                                 = Result<SharedPtr<LoggingBackend>, LoggerStorageError>;
        LoggingStorage()                                 = default;
        LoggingStorage(const LoggingStorage&)            = delete;
        LoggingStorage(LoggingStorage&&)                 = delete;
        LoggingStorage& operator=(const LoggingStorage&) = delete;
        LoggingStorage& operator=(LoggingStorage&&)      = delete;

        void add_backend(const String& name, SharedPtr<LoggingBackend> backend);
        void remove_backend(const String& name);
        ResultType get(StringView name);
    };
    static LoggingStorage& store() {
        static LoggingStorage store{};
        return store;
    }
    public:
    template <DerivedFrom<LoggingBackend> T>
    static void register_default_logger(T&& backend) {
        store().add_backend("default"_s, SharedPtr<LoggingBackend>{ Forward<T>(backend) });
    }
    template <DerivedFrom<LoggingBackend> T>
    static void register_named_logger(String name, T&& backend) {
        store().add_backend(name, SharedPtr<LoggingBackend>{ Forward<T>(backend) });
    }
    static LoggingStorage::ResultType get_named_logger(StringView name);
    static LoggingStorage::ResultType get_default_logger();
#ifdef __INTELLISENSE__
    template <typename... Args>
    requires(... && Printable<RemoveReferenceT<Args>>)
    static void log(StringView logger_name, LogLevel level, StringView str, Args&&... args) {
        auto formatted_string = Printer::format(move(str), Forward<Args>(args)...);
        if (auto it = store().get(logger_name); it.is_ok()) {
            auto backend = it.to_ok();
            backend->log(level, formatted_string);
        }
    }
    template <typename... Args>
    requires(... && Printable<RemoveReferenceT<Args>>)
    static void log(LogLevel level, StringView str, Args&&... args) {
        auto formatted_string = Printer::format(move(str), Forward<Args>(args)...);
        auto& sync_store      = Logger::store();
        sync_store.m_store.with_lock([formatted_string = move(formatted_string), level](LoggingStore& store) {
            for (auto& v : store) { v.val()->log(level, formatted_string); }
        });
    }
#else
    template <typename... Args>
    requires(... && Printable<RemoveReferenceT<Args>>)
    static void log(StringView logger_name, LogLevel level, FormatString<sizeof...(Args)> str, Args&&... args) {
        auto formatted_string = Printer::format(move(str), Forward<Args>(args)...);
        if (auto it = store().get(logger_name); it.is_ok()) {
            auto backend = it.to_ok();
            backend->log(level, formatted_string);
        }
    }
    template <typename... Args>
    requires(... && Printable<RemoveReferenceT<Args>>)
    static void log(LogLevel level, FormatString<sizeof...(Args)> str, Args&&... args) {
        auto formatted_string = Printer::format(move(str), Forward<Args>(args)...);
        auto& sync_store      = Logger::store();
        sync_store.m_store.with_lock([formatted_string = move(formatted_string), level](LoggingStore& store) {
            for (auto& v : store) { v.val()->log(level, formatted_string); }
        });
    }
#endif
};
}    // namespace ARLib
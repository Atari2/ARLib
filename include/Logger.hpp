#pragma once
#include "FlatMap.hpp"
#include "SharedPtr.hpp"
#include "Threading.hpp"
#include "Result.hpp"
#include "Stream.hpp"
#include "FormatString.hpp"
#include "Sync.hpp"
#include "StringLiteral.hpp"
namespace ARLib {
constexpr static inline uint8_t _newline_buffer[]{ '\n' };
constexpr static inline Span<const uint8_t> _newline_span{ _newline_buffer };
MAKE_FANCY_ENUM(LogLevel, uint8_t, Critical = 5, Error = 4, Warning = 3, Info = 2, Debug = 1, Trace = 0);
MAKE_FANCY_ENUM(LoggingError, uint8_t, FormatError, OutputError, OpenStreamError, LoggerNotFoundError);
constexpr StringView DefaultLogFormat = "%c[%n|%l - %u][%t]%r %m";
constexpr StringView DefaultLogFileFormat = "[%n|%l - %u][%t] %m";
class LoggingFormat {
    enum class LoggingFormatSpecifier {
        Message,
        LogLevel,
        Timestamp,
        ThreadId,
        DefaultColor,
        NoColor,
        LoggerName,
        LineBreak
    };
    using LoggingFormatPart = Variant<LoggingFormatSpecifier, String>;
    Vector<LoggingFormatPart> m_parts;
    public:
    LoggingFormat() = default;
    static Result<LoggingFormat, LoggingError> from_string(StringView format);
    String format_message(StringView message, LogLevel level, StringView logger_name) const;
};
class LoggingBackend {
    String m_name;
    LogLevel m_level;
    LoggingFormat m_format;
    friend class LoggingBackendTs;
    protected:
    String format_message(StringView message, LogLevel level) const;
    protected:
    LoggingBackend(String name, LogLevel level, LoggingFormat format) :
        m_name{ name }, m_level{ level }, m_format{ format } {}
    public:
    using LogResult = DiscardResult<LoggingError>;
    constexpr bool should_log(LogLevel level) const { return level >= m_level; }
    const String& name() const { return m_name; }
    virtual LogResult log(LogLevel level, StringView message) = 0;
    virtual bool supports_color() { return false; }
    virtual ~LoggingBackend() = default;
};
class LoggingBackendTs : public LoggingBackend {
    Mutex m_mutex;

    virtual LogResult _log_ts(LogLevel level, StringView message) = 0;
    protected:
    LoggingBackendTs(String name, LogLevel level, LoggingFormat format) :
        LoggingBackend{ name, level, format }, m_mutex{} {}
    public:
    LogResult log(LogLevel level, StringView message);
    virtual ~LoggingBackendTs() = default;
};
class ConsoleLogger : public LoggingBackend {
    ConsoleLogger(String name, LogLevel level, LoggingFormat format) : LoggingBackend{ name, level, format } {}
    public:
    static Result<SharedPtr<LoggingBackend>, LoggingError>
    create(String name, LogLevel level, StringView format = DefaultLogFormat) {
        TRY_SET(parsed_format, LoggingFormat::from_string(format));
        return SharedPtr<LoggingBackend>{
            new ConsoleLogger{ move(name), level, move(parsed_format) }
        };
    }
    LogResult log(LogLevel level, StringView message) override;
    bool supports_color() override { return true; }
};
class ConsoleLoggerTs : public LoggingBackendTs {
    LogResult _log_ts(LogLevel level, StringView message) override;
    ConsoleLoggerTs(String name, LogLevel level, LoggingFormat format) : LoggingBackendTs{ name, level, format } {}
    public:
    static Result<SharedPtr<LoggingBackend>, LoggingError>
    create(String name, LogLevel level, StringView format = DefaultLogFormat) {
        TRY_SET(parsed_format, LoggingFormat::from_string(format));
        return SharedPtr<LoggingBackend>{
            new ConsoleLoggerTs{ move(name), level, move(parsed_format) }
        };
    }
    bool supports_color() override { return true; }
};
template <DerivedFrom<BaseStream> T>
class StreamLogger : public LoggingBackend {
    UniquePtr<T> m_stream;
    friend class FileLogger;
    friend class BufferedFileLogger;
    friend class StringLogger;
    StreamLogger(String name, LogLevel level, LoggingFormat format, T&& stream) :
        LoggingBackend{ name, level, format }, m_stream{ move(stream) } {}
    public:
    template <DerivedFrom<BaseStream> S>
    static Result<SharedPtr<LoggingBackend>, LoggingError>
    create(String name, LogLevel level, S stream, StringView format = DefaultLogFormat) {
        TRY_SET(parsed_format, LoggingFormat::from_string(format));
        return SharedPtr<LoggingBackend>{
            new StreamLogger{ move(name), level, move(parsed_format), move(stream) }
        };
    }
    LogResult log(LogLevel level, StringView message) override {
        if (!should_log(level)) return {};
        auto formatted_message = format_message(message, level);
        TRY(m_stream->write(formatted_message.view()).map_error([](auto&& e) { return LoggingError::OutputError; }));
        TRY(m_stream->write(_newline_span).map_error([](auto&& e) { return LoggingError::OutputError; }));
        return {};
    }
};
template <DerivedFrom<BaseStream> T>
class StreamLoggerTs : public LoggingBackendTs {
    UniquePtr<T> m_stream;
    friend class FileLoggerTs;
    friend class BufferedFileLoggerTs;
    friend class StringLoggerTs;
    LogResult _log_ts(LogLevel level, StringView message) override {
        if (!should_log(level)) return {};
        auto formatted_message = format_message(message, level);
        TRY(m_stream->write(formatted_message.view()).map_error([](auto&& e) { return LoggingError::OutputError; }));
        TRY(m_stream->write(_newline_span).map_error([](auto&& e) { return LoggingError::OutputError; }));
        return {};
    }
    StreamLoggerTs(String name, LogLevel level, LoggingFormat format, T&& stream) :
        LoggingBackendTs{ name, level, format }, m_stream{ move(stream) } {}
    public:
    template <DerivedFrom<BaseStream> S>
    static Result<SharedPtr<LoggingBackend>, LoggingError>
    create(String name, LogLevel level, S stream, StringView format = DefaultLogFormat) {
        TRY_SET(parsed_format, LoggingFormat::from_string(format));
        return SharedPtr<LoggingBackend>{
            new StreamLoggerTs{ move(name), level, move(parsed_format), move(stream) }
        };
    }
};
class FileLogger : public StreamLogger<FileStream> {
    String m_file_path;
    FileLogger(String name, LogLevel level, LoggingFormat format, String file_path, FileStream&& stream) :
        StreamLogger<FileStream>{ name, level, format, move(stream) }, m_file_path{ move(file_path) } {}
    public:
    static Result<SharedPtr<LoggingBackend>, LoggingError>
    create(String name, LogLevel level, String file_path, StringView format = DefaultLogFileFormat) {
        TRY_SET(parsed_format, LoggingFormat::from_string(format));
        FileStream stream{ file_path };
        TRY(stream.open().map_error([](auto&& e) { return LoggingError::OpenStreamError; }));
        return SharedPtr<LoggingBackend>{
            new FileLogger{ move(name), level, move(parsed_format), move(file_path), move(stream) }
        };
    }
};
class BufferedFileLogger : public StreamLogger<BufferedFileStream> {
    String m_file_path;
    BufferedFileLogger(String name, LogLevel level, LoggingFormat format, String file_path, BufferedFileStream&& stream) :
        StreamLogger<BufferedFileStream>{ name, level, format, move(stream) }, m_file_path{ move(file_path) } {}
    public:
    static Result<SharedPtr<LoggingBackend>, LoggingError>
    create(String name, LogLevel level, String file_path, StringView format = DefaultLogFileFormat) {
        TRY_SET(parsed_format, LoggingFormat::from_string(format));
        BufferedFileStream stream{ file_path };
        TRY(stream.open().map_error([](auto&& e) { return LoggingError::OpenStreamError; }));
        return SharedPtr<LoggingBackend>{
            new BufferedFileLogger{ move(name), level, move(parsed_format), move(file_path), move(stream) }
        };
    }
};
class StringLogger : public StreamLogger<StringStream> {
    StringLogger(String name, LogLevel level, LoggingFormat format) :
        StreamLogger<StringStream>{ name, level, format, StringStream{} } {}
    public:
    static Result<SharedPtr<LoggingBackend>, LoggingError>
    create(String name, LogLevel level, StringView format = DefaultLogFileFormat) {
        TRY_SET(parsed_format, LoggingFormat::from_string(format));
        return SharedPtr<LoggingBackend>{
            new StringLogger{ move(name), level, move(parsed_format) }
        };
    }
    String output() const { return m_stream->str(); }
};
class FileLoggerTs : public StreamLoggerTs<FileStream> {
    String m_file_path;
    FileLoggerTs(String name, LogLevel level, LoggingFormat format, String file_path, FileStream&& stream) :
        StreamLoggerTs<FileStream>{ name, level, format, move(stream) }, m_file_path{ move(file_path) } {
    }
    public:
    static Result<SharedPtr<LoggingBackend>, LoggingError>
    create(String name, LogLevel level, String file_path, StringView format = DefaultLogFileFormat) {
        TRY_SET(parsed_format, LoggingFormat::from_string(format));
        FileStream stream{ file_path };
        TRY(stream.open().map_error([](auto&& e) { return LoggingError::OpenStreamError; }));
        return SharedPtr<LoggingBackend>{
            new FileLoggerTs{ move(name), level, move(parsed_format), move(file_path), move(stream) }
        };
    }
};
class BufferedFileLoggerTs : public StreamLoggerTs<BufferedFileStream> {
    String m_file_path;
    BufferedFileLoggerTs(
    String name, LogLevel level, LoggingFormat format, String file_path, BufferedFileStream&& stream
    ) : StreamLoggerTs<BufferedFileStream>{ name, level, format, move(stream) }, m_file_path{ move(file_path) } {}
    public:
    static Result<SharedPtr<LoggingBackend>, LoggingError>
    create(String name, LogLevel level, String file_path, StringView format = DefaultLogFileFormat) {
        TRY_SET(parsed_format, LoggingFormat::from_string(format));
        BufferedFileStream stream{ file_path };
        TRY(stream.open().map_error([](auto&& e) { return LoggingError::OpenStreamError; }));
        return SharedPtr<LoggingBackend>{
            new BufferedFileLoggerTs{ move(name), level, move(parsed_format), move(file_path), move(stream) }
        };
    }
};
class StringLoggerTs : public StreamLoggerTs<StringStream> {
    StringLoggerTs(String name, LogLevel level, LoggingFormat format) :
        StreamLoggerTs<StringStream>{ name, level, format, StringStream{} } {}
    public:
    static Result<SharedPtr<LoggingBackend>, LoggingError>
    create(String name, LogLevel level, StringView format = DefaultLogFileFormat) {
        TRY_SET(parsed_format, LoggingFormat::from_string(format));
        return SharedPtr<LoggingBackend>{
            new StringLoggerTs{ move(name), level, move(parsed_format) }
        };
    }
    String output() const { return m_stream->str(); }
};
using LoggingStore = FlatMap<String, SharedPtr<LoggingBackend>>;
class Logger {
    class LoggingStorage {
        friend Logger;
        SyncData<LoggingStore> m_store{ {} };
        using ResultType                                 = Result<SharedPtr<LoggingBackend>, LoggingError>;
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
    static void register_logger(SharedPtr<LoggingBackend> backend) {
        auto name_copy = backend->name();
        store().add_backend(move(name_copy), move(backend));
    }
    static LoggingStorage::ResultType get_named_logger(StringView name);
    static LoggingStorage::ResultType get_default_logger();
#ifdef __INTELLISENSE__
    #define LOGGER_STRING_PARAM_TYPE StringView
#else
    #define LOGGER_STRING_PARAM_TYPE FormatString<sizeof...(Args)>
#endif
    template <typename... Args>
    requires(... && Printable<RemoveReferenceT<Args>>)
    static LoggingBackend::LogResult
    log(StringView logger_name, LogLevel level, LOGGER_STRING_PARAM_TYPE str, Args&&... args) {
        auto formatted_string = Printer::format(move(str), Forward<Args>(args)...);
        if (auto it = store().get(logger_name); it.is_ok()) {
            auto backend = it.to_ok();
            TRY(backend->log(level, formatted_string));
        }
        return {};
    }
    template <typename... Args>
    requires(... && Printable<RemoveReferenceT<Args>>)
    static LoggingBackend::LogResult log(LogLevel level, LOGGER_STRING_PARAM_TYPE str, Args&&... args) {
        auto formatted_string = Printer::format(move(str), Forward<Args>(args)...);
        auto& sync_store      = Logger::store();
        return sync_store.m_store.with_lock(
        [formatted_string = move(formatted_string), level](LoggingStore& store) -> LoggingBackend::LogResult {
            for (auto& v : store) { TRY(v.val()->log(level, formatted_string)); }
            return {};
        }
        );
    }
};
}    // namespace ARLib
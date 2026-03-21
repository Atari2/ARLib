#include "Logger.hpp"
#include "Chrono.hpp"
namespace ARLib {
namespace AnsiEsc {
    constexpr auto escape   = "\x1b["_l;
    constexpr auto reset    = escape + "0m";
    constexpr auto red      = escape + "31m";
    constexpr auto green    = escape + "32m";
    constexpr auto yellow   = escape + "33m";
    constexpr auto blue     = escape + "34m";
    constexpr auto white    = escape + "37m";
    constexpr auto defaultc = escape + "39m";

    constexpr StringView color_map[]{
        defaultc.view(),    // trace
        defaultc.view(),    // debug
        blue.view(),        // info
        yellow.view(),      // warning
        red.view(),         // error
        red.view()          // critical
    };
}    // namespace AnsiEsc
Result<LoggingFormat, LoggingError> LoggingFormat::from_string(StringView format) {
    auto current_literal = format.begin();
    LoggingFormat fmt{};
    bool in_specifier = false;
    for (auto it = format.begin(); it != format.end(); ++it) {
        const char c = *it;
        if (in_specifier) {
            switch (c) {
                case '%':
                    // %% -> literal %
                    fmt.m_parts.emplace("%");
                    break;
                case 'm':
                case 'M':
                    fmt.m_parts.emplace(LoggingFormatSpecifier::Message);
                    break;
                case 'l':
                    fmt.m_parts.emplace(LoggingFormatSpecifier::LogLevel);
                    break;
                case 'L':
                    fmt.m_parts.emplace(LoggingFormatSpecifier::LogLevelShort);
                    break;
                case 'u':
                case 'U':
                    fmt.m_parts.emplace(LoggingFormatSpecifier::Timestamp);
                    break;
                case 't':
                case 'T':
                    fmt.m_parts.emplace(LoggingFormatSpecifier::ThreadId);
                    break;
                case 'c':
                case 'C':
                    fmt.m_parts.emplace(LoggingFormatSpecifier::DefaultColor);
                    break;
                case 'N':
                case 'n':
                    fmt.m_parts.emplace(LoggingFormatSpecifier::LoggerName);
                    break;
                case 'r':
                case 'R':
                    fmt.m_parts.emplace(LoggingFormatSpecifier::NoColor);
                    break;
                case 'b':
                case 'B':
                    fmt.m_parts.emplace(LoggingFormatSpecifier::LineBreak);
                    break;
                default:
                    return LoggingError::FormatError;
            }
            in_specifier    = false;
            current_literal = it + 1;
        } else {
            if (c == '%') {
                in_specifier = true;
                StringView literal{ current_literal, it };
                if (!literal.empty()) { fmt.m_parts.emplace(literal.str()); }
                current_literal = it + 1;
            }
        }
    }
    StringView literal{ current_literal, format.end() };
    fmt.m_parts.emplace(literal.str());
    return fmt;
}
String LoggingFormat::format_message(StringView message, LogLevel level, StringView logger_name) const {
    String output{};
    for (const auto& specifier : m_parts) {
        if (specifier.contains_type<LoggingFormatSpecifier>()) {
            const auto& spec = specifier.get<LoggingFormatSpecifier>();
            switch (spec) {
                case LoggingFormatSpecifier::Message:
                    output.append(message);
                    break;
                case LoggingFormatSpecifier::LogLevel:
                    output.append(enum_to_str_view(level));
                    break;
                case LoggingFormatSpecifier::LogLevelShort:
                    output.append(enum_to_str_view(level).first()); 
                    break;
                case LoggingFormatSpecifier::LoggerName:
                    output.append(logger_name);
                    break;
                case LoggingFormatSpecifier::Timestamp:
                    {
                        auto now = Date{ DateClock::now() };
                        output.append(now.to_string());
                    }
                    break;
                case LoggingFormatSpecifier::ThreadId:
                    {
                        auto tid = ThisThread::id();
                        output.append(PrintInfo{ tid }.repr());
                    }
                    break;
                case LoggingFormatSpecifier::DefaultColor:
                    output.append(AnsiEsc::color_map[from_enum(level)]);
                    break;
                case LoggingFormatSpecifier::NoColor:
                    output.append(AnsiEsc::reset.view());
                    break;
                case LoggingFormatSpecifier::LineBreak:
                    output.append('\n');
                    break;
            }
        } else {
            // literals
            const auto& literal = specifier.get<String>();
            output += literal;
        }
    }
    return output;
}
String LoggingBackend::format_message(StringView message, LogLevel level) const {
    return m_format.format_message(message, level, m_name);
}
LoggingBackend::LogResult LoggingBackendTs::log(LogLevel level, StringView message) {
    ScopedLock lock{ m_mutex };
    return this->_log_ts(level, message);
}
LoggingBackend::LogResult ConsoleLoggerTs::_log_ts(LogLevel level, StringView message) {
    if (!should_log(level)) return {};
    auto formatted_message = format_message(message, level);
    int ret                = ARLib::puts(formatted_message.data());
    return ret == EOF ? LogResult{ LoggingError::OutputError } : LogResult{ DefaultOk{} };
}
LoggingBackend::LogResult ConsoleLogger::log(LogLevel level, StringView message) {
    if (!should_log(level)) return {};
    auto formatted_message = format_message(message, level);
    int ret                = ARLib::puts(formatted_message.data());
    return ret == EOF ? LogResult{ LoggingError::OutputError } : LogResult{ DefaultOk{} };
}
void Logger::LoggingStorage::add_backend(const String& name, SharedPtr<LoggingBackend> backend) {
    m_store.with_lock([&name, backend = move(backend)](LoggingStore& map) { map.insert(name, move(backend)); });
}
void Logger::LoggingStorage::remove_backend(const String& name) {
    m_store.with_lock([&name](LoggingStore& map) { map.remove(name); });
}
Logger::LoggingStorage::ResultType Logger::LoggingStorage::get(StringView name) {
    auto store = m_store.lock();
    if (auto it = store->find(name); it != store->end()) {
        return (*it).val();
    } else {
        return LoggingError::LoggerNotFoundError;
    }
}
Logger::LoggingStorage::ResultType Logger::get_named_logger(StringView name) {
    return store().get(name);
}
Logger::LoggingStorage::ResultType Logger::get_default_logger() {
    return store().get("default"_sv);
}
}    // namespace ARLib
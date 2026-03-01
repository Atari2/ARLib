#include "Tuple.hpp"
#include "Printer.hpp"
#include "CharConv.hpp"
#include "SSOVector.hpp"
#include "BigInt.hpp"
#include "FileSystem.hpp"
#include "Logger.hpp"

using namespace ARLib;
int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
    auto console_logger = MUST(ConsoleLogger::create("default"_s, LogLevel::Trace, "%c%m %l%r test"));
    auto file_logger    = MUST(FileLoggerTs::create("file"_s, LogLevel::Info, "log.txt"_s));
    auto string_logger  = MUST(StringLogger::create("buffer"_s, LogLevel::Debug, DefaultLogFormat));
    Logger::register_logger(console_logger);
    Logger::register_logger(file_logger);
    Logger::register_logger(string_logger);
    for (size_t i = 0; i < 10; i++) { Logger::log_warning("Hello from {} {}."_sv, "ARLib", i); }
    auto logger = Logger::get_named_logger("buffer"_sv);
    if (logger.is_ok()) { Printer::print("String logger contents: {}", logger.to_ok().as<StringLogger>().output()); }
    console_logger->log_info("This is a test log message with value: {}"_sv, 42);
    file_logger->log_error("This is an error message with value: {}"_sv, 42);
    for (const auto& line : string_logger.as<StringLogger>().lines()) { Printer::print("Logged line: {}", line); }
    return 0;
}

#include "Tuple.hpp"
#include "Printer.hpp"
#include "CharConv.hpp"
#include "SSOVector.hpp"
#include "BigInt.hpp"
#include "FileSystem.hpp"
#include "Logger.hpp"
#include "DoubleFromChars.hpp"

using namespace ARLib;
int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
    //auto console_logger = MUST(ConsoleLogger::create("default"_s, LogLevel::Trace, "%c%m %l%r test"));
    //auto file_logger    = MUST(FileLoggerTs::create("file"_s, LogLevel::Info, "log.txt"_s));
    //auto string_logger  = MUST(StringLogger::create("buffer"_s, LogLevel::Debug, DefaultLogFormat));
    //Logger::register_logger(console_logger);
    //Logger::register_logger(file_logger);
    //Logger::register_logger(string_logger);
    //for (size_t i = 0; i < 10; i++) { Logger::log_warning("Hello from {} {}."_sv, "ARLib", i); }
    //auto logger = Logger::get_named_logger("buffer"_sv);
    //if (logger.is_ok()) { Printer::print("String logger contents: {}", logger.to_ok().as<StringLogger>().output()); }
    //console_logger->log_info("This is a test log message with value: {}"_sv, 42);
    //file_logger->log_error("This is an error message with value: {}"_sv, 42);
    //for (const auto& line : string_logger.as<StringLogger>().lines()) { Printer::print("Logged line: {}", line); }
    //constexpr const char* test  = "123456.1234";
    //constexpr const char* test2 = "0.0000000000000000000000000002";
    //constexpr size_t len        = strlen(test2);
    //auto val                    = DoubleFromChars(test, strlen(test));
    //auto val2                   = DoubleFromChars(test2, len);
    //Printer::print("{} {.32}", val, val2.to_ok());

    //String mv = "18,446,744,073,709,551,615"_s.replace(",", "");
    //BigInt bv{ mv };
    //BigInt bi{
    //    "1584830427832001978373428214706578240986387238183124667460474794597290057578846187414084558057027516086041384374216885097020014141"
    //};
    //auto db = bi.to_double_lossy();
    //Printer::print("{} {} {} {}", mv, bv, bv.fits(), db);

    auto format = LoggingFormat::from_string("%c{bright_red} ciao %c{0,0,0} %m%r").must();
    Printer::print("{}", format.format_message("hello world", LogLevel::Info, "test"));
    return 0;
}

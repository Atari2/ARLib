#include "Tuple.hpp"
#include "Printer.hpp"
#include "CharConv.hpp"
#include "SSOVector.hpp"
#include "BigInt.hpp"
#include "FileSystem.hpp"
#include "Logger.hpp"

using namespace ARLib;
int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
    Logger::register_default_logger(ConsoleLogger{ LogLevel::Trace });
    Logger::register_named_logger("file"_s, FileLogger{ LogLevel::Info, "log.txt"_s });
    Logger::register_named_logger("buffer"_s, StringLogger{ LogLevel::Debug });
    for (size_t i = 0; i < 10; i++) { Logger::log(LogLevel::Warning, "Hello from {} {}."_sv, "ARLib", i); }
    auto logger = Logger::get_named_logger("buffer"_sv);
    if (logger.is_ok()) { Printer::print("String logger contents: {}", logger.to_ok().as<StringLogger>().output()); }
    return 0;
}

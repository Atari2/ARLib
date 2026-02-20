#include "Tuple.hpp"
#include "Printer.hpp"
#include "CharConv.hpp"
#include "SSOVector.hpp"
#include "BigInt.hpp"
#include "FileSystem.hpp"
#include "Logger.hpp"

using namespace ARLib;
int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
    Logger::register_default_logger(SharedPtr{ new ConsoleLogger{ LogLevel::Debug } });
    return 0;
}

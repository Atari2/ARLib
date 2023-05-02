#include "Regex.hpp"
#include "Chrono.hpp"
#include "Random.hpp"
#include "Printer.hpp"
#include "File.hpp"

using namespace ARLib;
int main() {
    auto now  = DateClock::now();
    auto date = Date{ now };
    Printer::print("{}", date);
}
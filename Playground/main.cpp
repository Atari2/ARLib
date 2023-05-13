#include "Regex.hpp"
#include "Chrono.hpp"
#include "Random.hpp"
#include "Printer.hpp"
#include "File.hpp"

using namespace ARLib;
int main() {
    auto oldest_date = Date{ Instant::from_nanos(0_ns), Date::Timezone::No };
    Printer::print("{}", oldest_date);
}
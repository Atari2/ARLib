#include "CSVParser.hpp"
#include "File.hpp"
#include "FileSystem.hpp"
#include "Printer.hpp"

using namespace ARLib;
int main(int argc, char** argv) {
    CSVParser parser{ "test.txt"_p };
    parser.with_separator(':');
    parser.with_header(true);
    parser.open().must();
    auto rows = parser.read_all().must().iter().map(&CSVResult::must).collect<Vector>();
    Printer::print("{}", rows[0]["this"_sv].must());
    return 0;
}

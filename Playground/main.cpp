#include "CSVParser.hpp"
#include "File.hpp"
#include "FileSystem.hpp"
#include "Printer.hpp"
#include "Random.hpp"

using namespace ARLib;
int main(int argc, char** argv) {
    CSVParser parser{
        R"(this:barely:works
"a":"b
c":"d"
e:f:g
"h":"j":"k")"_s
    };
    // CSVParser parser{ "test.txt"_p };
    parser.with_separator(':');
    parser.with_header(true);
    parser.open().must();
    auto rows = parser.read_all().must().iter().map(&CSVResult::must).collect<Vector>();
    for (const auto& row : rows) {
        Printer::print(R"("{}" "{}" "{}")", row["this"_sv].must(), row["barely"_sv].must(), row["works"_sv].must());
    }
    auto v = Random::HardwareGen::random_64().must();
    Printer::print("{}", v);
    return 0;
}

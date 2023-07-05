#include "String.hpp"
#include "Variant.hpp"
#include "Vector.hpp"
#include "Array.hpp"
#include "Printer.hpp"
#include "CharConv.hpp"
#include "JSONParser.hpp"
#include "FileSystem.hpp"
using namespace ARLib;
int main() {
    Timer tmr{};
    auto j = MUST(JSON::Parser::from_file(R"(C:\Users\aless\Downloads\test.json)"_p));
    Printer::print("Parsed json file in {}", tmr.elapsed_since_last().to_common().millis());
    auto s = JSON::dump_json(j.root());
    Printer::print("Serialized json to string in {}", tmr.elapsed_since_last().to_common().millis());
    MUST(File::write_all("out.json"_p, s));
    Printer::print("Written string of length {} to file in {}", s.size(), tmr.elapsed_since_last().to_common().millis());
    return 0;
}
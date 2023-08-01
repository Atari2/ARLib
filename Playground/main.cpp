#include "String.hpp"
#include "Variant.hpp"
#include "Vector.hpp"
#include "Array.hpp"
#include "Printer.hpp"
#include "CharConv.hpp"
#include "JSONParser.hpp"
#include "FileSystem.hpp"
#include "Console.hpp"
#include "ArgParser.hpp"
using namespace ARLib;
int main(int argc, char** argv) {
    auto parser = ArgParser{ argc, argv };
    Vector<int> vec{};
    parser.add_option("--nums", "NUMBER LIST", "A list of numbers", vec);
    auto ok = parser.parse();
    if (ok.is_error()) {
        Printer::print("{}", ok.to_error());
    } else {
        Printer::print("{}", vec);
        Printer::print("{}", parser.get<Vector<int>>("--nums"));
    }
    Printer::print("{}", parser.help_string());
    auto j      = JSON::Parser::parse(R"([1, 2, 3])"_sv).must();
    auto o      = JSON::Object{};
    auto str    = Console::getline();
    o[str]      = 1;
    auto& arr   = j.root();
    arr[0]      = o;
    arr[0][str] = Console::getnumber<int64_t>().must();
    j.serialize_to_file("test.json"_p);
    return 0;
}
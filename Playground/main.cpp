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
    return 0;
}
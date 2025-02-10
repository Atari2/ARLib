#include "Vector.hpp"
#include "GenericView.hpp"
#include "String.hpp"
#include "CharConv.hpp"
#include "Printer.hpp"
#include "JSONParser.hpp"
#include "Tuple.hpp"
#include "ArgParser.hpp"

using namespace ARLib;
int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
    ArgParser parser{ argc, argv };
    Path p{};
    parser.add_option({ "-p", "--path" }, "PATH", "Path to file", p);
    parser.parse().must();
    Printer::print("{}", p);
    return 0;
}

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
    auto path = windows_build ? R"(C:\Users\aless\Downloads\test.json)"_p : R"(/home/alessio/test.json)"_p;
    auto p = MUST(JSON::Parser::from_file(path));
    return 0;
}
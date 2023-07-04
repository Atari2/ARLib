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
    auto j = MUST(JSON::Parser::parse(R"({"hello": [1, { "a": true }, "hello"]})"_sv));
    MUST(j.serialize_to_file("test.json"_p));
    return 0;
}
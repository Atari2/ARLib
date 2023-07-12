#include "String.hpp"
#include "Variant.hpp"
#include "Vector.hpp"
#include "Array.hpp"
#include "Printer.hpp"
#include "CharConv.hpp"
#include "JSONParser.hpp"
#include "FileSystem.hpp"
#include "Console.hpp"
using namespace ARLib;
int main() {
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
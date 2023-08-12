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
    auto s = FlatMap<String, int>{};
    auto s2 = FlatMap<String, String>{};
    s.insert("hello world"_s, 2);
    s.get_or_default("hello world"_s) += 1;
    s.get_or_default("testing") += 1234;
    s2.get_or_default("piercarlo") += "testing"_s;
    s2.get_or_insert("piercarlo", "hello world"_s);
    s2.get_or_insert("asdf", "hello world"_s);
    Printer::print("{}", s["hello world"_sv]);
    Printer::print("{} {}", s["testing"], s2["piercarlo"]);
    Printer::print("{}", s2["asdf"]);
    return 0;
}
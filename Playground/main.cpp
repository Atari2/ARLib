#include "../BigInt.h"
#include "../CharConv.h"
#include "../File.h"
#include "../Printer.h"
#include "../Variant.h"

#include <cstdio>

using namespace ARLib;
int main() {
    BigInt a{99};
    BigInt b{99};
    auto c = a + b;
    Printer::print("{}", c);
    Vector<Variant<String, int>> vars{10, "hello world"};
    vars.for_each(
    [](const auto& vsi) { visit(vsi, [](const auto& e) { Printer::print("Variant contains: {}", e); }); });
    vars.for_each([](const auto& vsi) { vsi.visit([](const auto& e) { Printer::print("Variant contains: {}", e); }); });
    File file{"HelloWorld.txt"_s};
    file.open(OpenFileMode::Read);
    auto res = file.read_all();
    Printer::print("{} {}", res, file.size());
}
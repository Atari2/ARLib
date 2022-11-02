#include "../CharConv.h"
#include "../Printer.h"
#include "../Variant.h"
using namespace ARLib;
int main() {
    Vector<Variant<String, int>> vars{10, "hello world"};
    vars.for_each([](const auto& vsi) { visit(vsi, [](const auto& e) { Printer::print("Variant contains: {}", e); }); });
    vars.for_each([](const auto& vsi) { vsi.visit([](const auto& e) { Printer::print("Variant contains: {}", e); }); });
}
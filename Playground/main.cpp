#include "../CharConv.h"
#include "../Printer.h"
#include "../String.h"
#include "../Pair.h"

using namespace ARLib;

int main() {
    Pair<String, int> pair{"hello"_s, 10};
    Printer::print("{}", pair);
    return 0;
}
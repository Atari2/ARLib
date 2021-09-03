#include "../Array.h"
#include "../CharConv.h"
#include "../Pair.h"
#include "../Printer.h"
#include "../String.h"
#include "../Tuple.h"
#include "../Variant.h"
#include "../Vector.h"

using namespace ARLib;

int main() {
    Pair<String, int> pair{"hello"_s, 10};
    Variant<String, int, Pair<String, int>, Vector<int>> variant{};
    Tuple<String, int, size_t> tup{"hello"_s, 10, 1000};
    Array<int, 4> arr{1, 2, 3, 4};
    Printer::print("{}, {}, {}, {}", pair, variant, tup, arr);
    return 0;
}
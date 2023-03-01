#include "../Printer.h"
#include "../String.h"
#include "../Vector.h"
#include "../Enumerate.h"
#include "../JSONParser.h"
#include "../FileSystem.h"
#include "../Set.h"
#include "../List.h"
#include "../Span.h"
#include "../CharConv.h"
#include "../SortedVector.h"
#include "../Array.h"

using namespace ARLib;
int main() {
    Vector<int> vec = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    auto span       = vec.span();
    auto span2      = span.subspan(2, 5);
    auto span3      = span.subspan(123123, 1234123);
    Array arr       = { "Hello"_sv, "World"_sv, "How"_sv, "Are"_sv, "You"_sv };
    auto span4      = arr.span().subspan(1, 3);
    auto span5      = GenericView{ arr }.span().subspan(1, 3);
    Printer::print("{}", span4 == span5);
    Printer::print("Span size: {}", span.size());
    Printer::print("Span2 size: {}", span2.size());
    Printer::print("Span3 size: {}", span3.size());
    Printer::print("Span4 size: {}", span4.size());
    for (auto [i, val] : enumerate(span)) { Printer::print("span[{}]: {}", i, val); }
    for (auto [i, val] : enumerate(span2)) { Printer::print("span[{}] {}", i, val); }
    for (auto [i, val] : enumerate(span3)) { Printer::print("span[{}] {}", i, val); }
    for (auto [i, val] : enumerate(span4)) { Printer::print("span[{}] {}", i, val); }
}
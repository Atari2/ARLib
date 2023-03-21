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
#include "../BetterResult.h"
#include <concepts>

using namespace ARLib;
int main() {
    vnext::Result r{ 1 };
    vnext::Result<int> r2{ vnext::BacktraceError{ "hello"_s } };
    HARD_ASSERT(r.is_ok(), "1");
    HARD_ASSERT(!r.is_error(), "2");
    HARD_ASSERT(r.ok_value() == 1, "3");
    Printer::print("{} {}", r, r2);
}
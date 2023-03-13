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
#include "../cstdio_compat.h"
using namespace ARLib;
int main() {
    Array expected_from_en{
        Pair{0_sz,  1.0},
        Pair{ 1_sz, 2.0},
        Pair{ 2_sz, 3.0},
        Pair{ 3_sz, 4.0},
        Pair{ 4_sz, 5.0}
    };
    for (const auto& [exp_i, exp_v, act_i, act_v] : "1\n2\n3\n\n4\n\n5"_sv.split("\n")
                                                    .iter()
                                                    .filter(&StringView::size)
                                                    .map(StrViewToDouble)
                                                    .enumerate()
                                                    .zip(expected_from_en)) {
        Printer::print("{} {} {} {}", exp_i, exp_v, act_i, act_v);
    }
    Pair<Tuple<int, double>, int> p{ make_tuple(1, 2.0), 1 };
    auto&& [a, b, c] = Pair{ make_tuple(1, 2.0), 1 };
}

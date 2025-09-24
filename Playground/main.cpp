#include "Tuple.hpp"
#include "Printer.hpp"
#include "CharConv.hpp"
#include "SSOVector.hpp"
#include "BigInt.hpp"

using namespace ARLib;
int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
    auto f2 = BigInt{ "123456781234567891234879169467981276392189732178937891237928173981239812219873218973"_s };
    auto g2 = BigInt{ "12837127389712389123891738127317892312987389217"_s };
    auto d = f2 / g2;
    return 0;
}

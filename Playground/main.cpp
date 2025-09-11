#include "Tuple.hpp"
#include "Printer.hpp"
#include "CharConv.hpp"

using namespace ARLib;
int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
    auto tup = Tuple("This is a beatiful string"_s);
    auto tup2 = Tuple("This is another beatiful string"_s);
    const auto v1 = move(tup).flatten().get<0>();
    const auto v2   = Tuple("This is a beatiful string"_s).flatten().get<0>();
    const auto& v3 = tup2.get<0>();
    Printer::print(R"("{}" "{}" "{}")", v1, v2, tup.get<0>());
    Printer::print(R"("{}" "{}" "{}")", v3, tup2.get<0>(), &v3 == &tup2.get<0>());
    return 0;
}

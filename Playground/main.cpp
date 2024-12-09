#include "Vector.hpp"
#include "GenericView.hpp"
#include "String.hpp"
#include "CharConv.hpp"
#include "Printer.hpp"
#include "JSONParser.hpp"
#include "Tuple.hpp"

using namespace ARLib;
int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {

    Tuple<Tuple<Tuple<int, float>, Pair<Vector<int>, Vector<float>>>, Pair<String, StringView>> tup{};
    auto&& [i1, f1, vi, vf, s, sv] = flatten_tuple(tup);
    return 0;
}

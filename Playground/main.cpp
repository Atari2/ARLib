#include "Vector.hpp"
#include "GenericView.hpp"
#include "String.hpp"
#include "CharConv.hpp"
#include "Printer.hpp"
#include "JSONParser.hpp"

using namespace ARLib;
int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
    Vector<String> vec{ "123"_s, "asd"_s, "234"_s };
    auto coll = vec.iter().filter_map([](const auto& v) { return StrToInt(v).optional(); }).collect<Vector>();
    Printer::print("{}", coll);
    auto val = R"( { "hello": "world", "array": [1, 2, 3, 4, 5], "object": { "key": "value" } } )"_json;
    Printer::print("{}", val);
    return 0;
}

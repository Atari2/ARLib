#include "../Printer.h"
#include "../CharConv.h"
#include "../HashMap.h"
#include "../String.h"
#include "../Vector.h"
#include "../Enumerate.h"
using namespace ARLib;
int main() {
    String str{ "ABCDEFG" };
    Vector<int> vec{ 1, 2, 3 };
    Vector<String> vec2{"FOO"_s, "BAR"_s, "BAZ"_s};
    HashMap<String, String> map{};
    map.add("Hello"_s, "World"_s);
    map.add("World"_s, "Hello"_s);
    map.add("Foo"_s, "Bar"_s);
    for (const auto& [a, b, c, v] : zip(str, vec, vec2, map)) { Printer::print("{} {} {} {}", a, b, c, v); }
    for (const auto& [i, t]: enumerate(zip(str, vec, vec2, map))) { Printer::print("{} -> {}", i, t); }
}
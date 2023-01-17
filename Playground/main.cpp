#include "../Printer.h"
#include "../CharConv.h"
#include "../Array.h"
#include "../GenericView.h"
using namespace ARLib;
int main() {
    auto r = 
    Array{ 1, 2, 3, 4, 5 }
    .enumerate()
    .filter([](auto e) { auto [i, v] = e; return i % 2 == 0; })
    .map([](auto e) { auto [i, v] = e; return IntToStr(v); })
    .collect<Vector<String>>();

    const Vector<int> v{ 1, 20, 3, 40, 5 };
    auto r2 = v
    .view()
    .map([](int i) { return IntToStr(i); })
    .filter([](const String& s) { return s.size() % 2 == 0;})
    .collect<Vector<String>>();

    Printer::print("{} {}", r, r2);
}
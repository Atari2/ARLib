#include "Regex.hpp"
#include "Printer.hpp"
#include "Matrix.hpp"
#include "Chrono.hpp"

using namespace ARLib;
int main(int argc, char** argv) {
    auto re = "Hello (.*) World!"_re;
    Printer::print("{}", re);
    Vector<String> str{};
    v2::Variant<int, String, double> v;
    fill_with(str, 10, "hello");
    for (const auto& s : str) { Printer::print("{}", s); }
    constexpr auto secs = (1_sec).to<Millis>() + (2_ms).to<Nanos>();
    return 0;
}

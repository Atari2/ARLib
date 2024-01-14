#include "Regex.hpp"
#include "Printer.hpp"
#include "Matrix.hpp"

using namespace ARLib;
int main(int argc, char** argv) {
    auto re = "Hello (.*) World!"_re;
    Printer::print("{}", re);
    return 0;
}

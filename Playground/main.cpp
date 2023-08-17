#include "CSVParser.hpp"
#include "File.hpp"
#include "Filesystem.hpp"
#include "Printer.hpp"

using namespace ARLib;
int main(int argc, char** argv) {
    File f{ "test.txt"_p };
    f.open(OpenFileMode::Read);
    bool eof_reached = false;
    for (size_t i = 0; !eof_reached; ++i) {
        auto s = f.read_line(eof_reached).must();
        Printer::print("Line {}: {}", i, s.size());
        Printer::print("{}", s);
    }
    return 0;
}
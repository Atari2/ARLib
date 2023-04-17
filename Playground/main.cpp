#include "Regex.hpp"

using namespace ARLib;
int main() {
    Regex re = R"(hello ([\s(\d)]*)? (\w .+) world)"_re;
}
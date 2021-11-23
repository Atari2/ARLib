#include "../Printer.h"
#include "../BigInt.h"

using namespace ARLib;

int main() {
    auto a = BigInt{"12389123908"_s};
    auto b = BigInt{"-983458171238123"_s};
    auto c = BigInt{"875679183741987"_s};
    auto d = BigInt{"-1238767812763"_s};
    auto s = BigInt{"123456"_s};
    Printer::print("Multiplication test: {} / {}", s * s, BigInt{-1} + BigInt{1});
    auto f = BigInt{2234};
    auto g = BigInt{4321};
    auto f2 = BigInt{-234};
    auto g2 = BigInt{321};
    Printer::print("{} {}", g / f, g2 / f2);
    // Printer::print("Subtraction test: {} {} {} {} {} {}", a - b, b - a, a - c, c - a, b - d, d - b);
    // Printer::print("Sumation test: {} {} {} {} {} {}", a + b, b + a, a + c, c + a, b + d, d + b);
    return 0;
}
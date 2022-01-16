#include "../CharConv.h"
#include "../Conversion.h"
#include "../FixedMatrix.h"
#include "../Matrix.h"
#include "../Printer.h"
#include "../PriorityQueue.h"
#include "../Random.h"
using namespace ARLib;

int main() {
    auto m1 = FixedMatrix2D<int, 4, 2>::random(100);
    auto m2 = FixedMatrix2D<int, 2, 2>::random(100);
    auto cols = FixedMatrix2D<int, 1, 4>{1, 2, 3, 4};
    auto m4 = Matrix2D<int>{10, 10};
    auto m3 = cols[0] * m1;
    Printer::print("{}", m4);
    return 0;
}
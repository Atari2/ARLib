#include "../FixedMatrix.h"
#include "../Matrix.h"
#include "../Printer.h"
#include "../CharConv.h"
using namespace ARLib;

int main() {
    auto m1 = FixedMatrix2D<4, 2>::random(100);
    auto m2 = FixedMatrix2D<2, 2>::random(100);
    auto cols = FixedMatrix2D<1, 4>{1.0, 2.0, 3.0, 4.0};
    auto m4 = Matrix2D{10, 10};
    double vect[] = {82, 15, 66, 71, 44, 27, 75, 84, 35, 7,  91, 98, 3,  3,  38, 68, 25, 25, 83, 5,
                     12, 96, 85, 27, 77, 66, 51, 82, 59, 53, 92, 49, 94, 4,  80, 16, 70, 24, 55, 78,
                     63, 80, 68, 9,  18, 12, 89, 93, 92, 94, 9,  14, 76, 83, 49, 50, 96, 35, 28, 13,
                     28, 42, 75, 70, 45, 96, 55, 19, 76, 57, 55, 92, 39, 32, 65, 34, 14, 25, 76, 47,
                     96, 80, 66, 95, 71, 59, 15, 62, 38, 1,  97, 96, 17, 3,  76, 22, 26, 47, 57, 34};
    auto m5 = FixedMatrix2D<10, 10>{};
    for (size_t i = 0; i < 10; i++) {
        for (size_t j = 0; j < 10; j++) {
            m4[{j, i}] = vect[i * 10 + j];
            // m5[{j, i}] = vect[i * 10 + j];
        }
    }

    // m4.det() expected => -3715391934961051648.000000
    Printer::print("{}\n{}\n{}", m4, m4.det(), m4.inv());
    return 0;
}
#include "../CharConv.h"
#include "../Conversion.h"
#include "../Matrix.h"
#include "../Printer.h"
#include "../PriorityQueue.h"
#include "../Random.h"
using namespace ARLib;

int main() {
    double m[][4] = {{8.1472, 6.3236, 9.5751, 9.5717},
                     {9.0579, 0.9754, 9.6489, 4.8538},
                     {1.2699, 2.7850, 1.5761, 8.0028},
                     {9.1338, 5.4688, 9.7059, 1.4189}};
    Matrix2D<double, 4> matrix{m};
    matrix.inv();
    return 0;
}
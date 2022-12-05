#include "../Assertion.h"
#include "../CharConv.h"
#include "../NumberTraits.h"
#include "../Printer.h"
#include "../PrintfImpl.h"
#include "../Utility.h"
#include "../SSOVector.h"
#include <inttypes.h>
#include <stdio.h>

using namespace ARLib;
void double_tests() {
    double v = 25.65;

    ::printf("%.12f %F %g %G %.3e %E\n", v, v, v, v, v, v);
    ARLib::printf("%.12f %F %g %G %.3e %E\n", v, v, v, v, v, v);
}
#define PRINT_CHECK(expr) Printer::print(#expr ": {}", expr);
int main() {
    double val    = 1234.1234;
    const char* s = "my name is alessio";
    int hex       = 0x50;
    String buf    = "狗"_s;
    String s2     = "smör"_s;
    PRINT_CHECK("abababc"_s.index_of("ababc"));
    PRINT_CHECK("åäåäåäö"_s.index_of("åäåäö"));
    PRINT_CHECK(s2.index_of("ö"));
    PRINT_CHECK(buf.index_of("狗"));
    ARLib::printf("%hhx %llX\n", 10, 1235128937ll);
    ::printf("%hhx %llX\n", 10, 1235128937ll);
    double_tests();
    // int arlib_pn = 0;
    // int std_pn = 0;
    // ARLib::int64_t int64val = NumberTraits<ARLib::int64_t>::max;
    // float_tests();
    // ::printf("std-printf  : Hello World %+.5A %+10.4f %n %% %s %#02o %#02" PRIX64 " %%\n", 50.0, val, &std_pn, s, hex,
    //          int64val);
    // _printf("arlib-printf: Hello World %+.5A %+10.4f %n %% %s %#02o %#02" PRIX64 " %%\n", 50.0, val, &arlib_pn, s, hex,
    //         int64val);
    // Printer::print("arlib %n: {}, std %n: {}", arlib_pn, std_pn);
}
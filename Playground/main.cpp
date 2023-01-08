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

int main() {
    char buffer1[1024];
    char buffer2[1024];
    double val    = 1234.1234;
    const char* s = "my name is alessio";
    int hex       = 0x50;
    double v = 25.65;
    ::sprintf(buffer1, "%.12f %F %g %G %.3e %E", v, v, v, v, v, v);
    ARLib::sprintf(buffer2, "%.12f %F %g %G %.3e %E", v, v, v, v, v, v);
    Printer::print("std: {}\nARLib: {}", buffer1, buffer2);
    int arlib_pn = 0;
    int std_pn = 0;
    ARLib::int64_t int64val = NumberTraits<ARLib::int64_t>::max;
    ::sprintf(buffer1, "Hello World %+.5A %+10.4f %n %% %s %#02o %#02" PRIX64 " %%", 50.0, val, &std_pn, s, hex, int64val);
    ARLib::sprintf(buffer2, "Hello World %+.5A %+10.4f %n %% %s %#02o %#02" PRIX64 " %%", 50.0, val, &arlib_pn, s, hex, int64val);
    Printer::print("std: {}\nARLib: {}", buffer1, buffer2);
    Printer::print("std: {}\nARLib: {}", std_pn, arlib_pn);
}
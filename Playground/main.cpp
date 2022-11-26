#include "../Assertion.h"
#include "../PrintfImpl.h"
#include "../NumberTraits.h"
#include <inttypes.h>

using namespace ARLib;
int main() {
    float val = 1234.1234;
    const char* s = "my name is alessio";
    int hex = 0x50;
    ARLib::int64_t int64val = NumberTraits<ARLib::int64_t>::max;
    ::printf("Hello World %+10.4f %% %s %#02o %#02" PRIX64 " %%\n", val, s, hex, int64val);
    HARD_ASSERT(printf_impl("Hello World %+10.4f %% %s %#02o %#02I64X %%\n", val, s, hex, int64val) == PrintfErrorCodes::Ok,
                "Wrong printf impl");
    HARD_ASSERT(printf_impl("Hello World %10+.4f %s %02X %02I64X %%\n", val, s, hex, int64val) ==
                PrintfErrorCodes::InvalidFormat,
                "Wrong printf impl");
}
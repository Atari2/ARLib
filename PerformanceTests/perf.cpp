#include "PerfTime.h"
#include "../cstdio_compat.h"

int main(int argc, char* argv[]) {
    TIMER_START()
    ARLib::printf("Hello World from ARLibPerformanceSuite\n");
    TIMER_END
}
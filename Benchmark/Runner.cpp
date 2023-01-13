#include "../CharConv.h"
#include "../HashMap.h"
#include <benchmark/benchmark.h>
#include "../Assertion.h"
#include "../PrintfImpl.h"
#include <inttypes.h>

using namespace ARLib;
static void BM_PrintfARLib(benchmark::State& state) {
    char buf[1024]{};
    for (auto _ : state) {
        double val              = 1234.1234;
        const char* s           = "my name is alessio";
        int hex                 = 0x50;
        ARLib::int64_t int64val = INT64_MAX;
        ARLib::sprintf(buf, "Hello World %+10.4f %% %s %#02o %#02I64X %%\n", val, s, hex, int64val);
        benchmark::ClobberMemory();
    }
}
static void BM_PrintfStd(benchmark::State& state) {
    char buf[1024]{};
    for (auto _ : state) {
        double val              = 1234.1234;
        const char* s           = "my name is alessio";
        int hex                 = 0x50;
        ARLib::int64_t int64val = INT64_MAX;
        ::sprintf(buf, "Hello World %+10.4f %% %s %#02o %#02" PRIX64 " %%\n", val, s, hex, int64val);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_PrintfStd);
BENCHMARK(BM_PrintfARLib);
BENCHMARK_MAIN();
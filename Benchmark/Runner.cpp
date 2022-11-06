#include "../CharConv.h"
#include "../HashMap.h"
#include <benchmark/benchmark.h>
#include "../Assertion.h"
#include "../PrintfImpl.h"
#include <inttypes.h>

using namespace ARLib;

static void BM_SomeFunction(benchmark::State& state) {
    for (auto _ : state) {
        float val = 1234.1234;
        const char* s = "my name is alessio";
        int hex = 0x50;
        ARLib::uint64_t int64val = UINT64_MAX;
        printf_impl("Hello World %+10.4f %% %s %#02o %#02I64X %%\n", val, s, hex, int64val);
        benchmark::ClobberMemory();
    }
}

static void BM_SomeFunctionStd(benchmark::State& state) {
    for (auto _ : state) {
        float val = 1234.1234;
        const char* s = "my name is alessio";
        int hex = 0x50;
        ARLib::uint64_t int64val = UINT64_MAX;
        ::printf("Hello World %+10.4f %% %s %#02o %#02" PRIX64 " %%\n", val, s, hex, int64val);
        benchmark::ClobberMemory();
    }
}

// BENCHMARK(BM_SomeFunctionStd);
BENCHMARK(BM_SomeFunction);
BENCHMARK_MAIN();
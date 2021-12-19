#include "../CharConv.h"
#include "../HashMap.h"
#include "../Random.h"
#include "../String.h"
#include <benchmark/benchmark.h>
#include <iostream>
#include <string>
#include <unordered_map>

using namespace ARLib;

static void BM_SomeFunction(benchmark::State& state) {
    auto rnd = Random::PCG::create();
    for (auto _ : state) {
        auto num = rnd.random();
        String str = IntToStr(num);
        benchmark::DoNotOptimize(str.data());
        benchmark::ClobberMemory();
    }
}

static void BM_SomeFunctionStd(benchmark::State& state) {
    auto rnd = Random::PCG::create();
    for (auto _ : state) {
        auto num = rnd.random();
        std::string str = std::to_string(num);
        benchmark::DoNotOptimize(str.data());
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_SomeFunctionStd);
BENCHMARK(BM_SomeFunction);
BENCHMARK_MAIN();
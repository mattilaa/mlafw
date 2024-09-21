#include <benchmark/benchmark.h>

#include "mlafw/timer.h"

static void BM_Test(benchmark::State& state)
{
    (void)state;
}

BENCHMARK(BM_Test);

BENCHMARK_MAIN();

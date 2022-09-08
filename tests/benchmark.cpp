#include <benchmark/benchmark.h>

#include "mlafw/mlatimer.h"

static void BM_Test(benchmark::State& state)
{
    (void)state;
}

BENCHMARK(BM_Test);

BENCHMARK_MAIN();

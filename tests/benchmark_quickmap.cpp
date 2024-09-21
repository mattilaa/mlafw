#include <benchmark/benchmark.h>
#include "mlafw/vectorquickmap.h"
#include "mlafw/arrayquickmap.h"
#include <random>
#include <string>
#include <unordered_map>

using namespace mla;

// Utility function to generate random strings
std::string random_string(std::size_t length) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(alphanum) - 2);
    std::string result;
    result.reserve(length);
    for (std::size_t i = 0; i < length; ++i) {
        result += alphanum[dis(gen)];
    }
    return result;
}

// Benchmark for QuickMapVector
static void BM_VectorQuickMap(benchmark::State& state) {
    const int num_operations = state.range(0);
    std::vector<std::string> keys;
    keys.reserve(num_operations);
    for (int i = 0; i < num_operations; ++i) {
        keys.push_back(random_string(10));
    }

    for (auto _ : state) {
        VectorQuickMap<std::string, int> map;
        for (int i = 0; i < num_operations; ++i) {
            map.insert(keys[i], i);
        }
        for (int i = 0; i < num_operations; ++i) {
            benchmark::DoNotOptimize(map.get(keys[i]));
        }
    }
}

// Benchmark for QuickMapArray
template<std::size_t Capacity>
static void BM_ArrayQuickMap(benchmark::State& state) {
    const int num_operations = state.range(0);
    std::vector<std::string> keys;
    keys.reserve(num_operations);
    for (int i = 0; i < num_operations; ++i) {
        keys.push_back(random_string(10));
    }

    for (auto _ : state) {
        ArrayQuickMap<std::string, int, Capacity> map;
        for (int i = 0; i < num_operations && i < Capacity; ++i) {
            map.insert(keys[i], i);
        }
        for (int i = 0; i < num_operations && i < Capacity; ++i) {
            benchmark::DoNotOptimize(map.get(keys[i]));
        }
    }
}

// New benchmark for std::unordered_map
static void BM_UnorderedMap(benchmark::State& state) {
    const int num_operations = state.range(0);
    std::vector<std::string> keys;
    keys.reserve(num_operations);
    for (int i = 0; i < num_operations; ++i) {
        keys.push_back(random_string(10));
    }

    for (auto _ : state) {
        std::unordered_map<std::string, int> map;
        for (int i = 0; i < num_operations; ++i) {
            map.insert({keys[i], i});
        }
        for (int i = 0; i < num_operations; ++i) {
            benchmark::DoNotOptimize(map.find(keys[i]));
        }
    }
}

// Register benchmarks
BENCHMARK(BM_VectorQuickMap)->Range(8, 4096);
BENCHMARK(BM_ArrayQuickMap<6191>)->Range(8, 4096);
BENCHMARK(BM_UnorderedMap)->Range(8, 4096);

BENCHMARK_MAIN();

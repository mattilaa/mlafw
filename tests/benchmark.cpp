#include <benchmark/benchmark.h>
#include <vector>
#include <memory>
#include <array>

// A simple class for demonstration
class MyClass {
public:
    MyClass(int val) : value(val) {}
    MyClass() = default;
    int getValue() const { return value; }
private:
    int value;
};

// Benchmark for non-pointer based vector
static void BM_VectorNonPointer(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<MyClass> vec;
        for (int i = 0; i < state.range(0); ++i) {
            vec.emplace_back(MyClass{i});
        }
        // Prevent optimization
        benchmark::DoNotOptimize(vec);
    }
}

// Benchmark for pointer based vector
static void BM_VectorPointer(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<std::unique_ptr<MyClass>> vec;
        for (int i = 0; i < state.range(0); ++i) {
            vec.emplace_back(std::make_unique<MyClass>(i));
        }
        // Prevent optimization
        benchmark::DoNotOptimize(vec);
    }
}

// Benchmark for non-pointer based vector
static void BM_ArrayNonPointer(benchmark::State& state) {
    std::array<MyClass, 1024*8> vec;
    for (auto _ : state) {
        for (int i = 0; i < state.range(0); ++i) {
            vec[i] = MyClass{i};
        }
        // Prevent optimization
        benchmark::DoNotOptimize(vec);
    }
}

// Benchmark for pointer based vector
static void BM_ArrayPointer(benchmark::State& state) {
    std::array<std::unique_ptr<MyClass>, 1024*8> vec;
    for (auto _ : state) {
        for (int i = 0; i < state.range(0); ++i) {
            vec[i] = (std::make_unique<MyClass>(i));
        }
        // Prevent optimization
        benchmark::DoNotOptimize(vec);
    }
}

// Register the benchmarks
BENCHMARK(BM_VectorNonPointer)->Range(8, 8<<10);
BENCHMARK(BM_VectorPointer)->Range(8, 8<<10);
BENCHMARK(BM_ArrayNonPointer)->Range(8, 8<<10);
BENCHMARK(BM_ArrayPointer)->Range(8, 8<<10);
BENCHMARK_MAIN();

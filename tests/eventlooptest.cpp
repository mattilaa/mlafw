#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <random>
#include <algorithm>
#include "mlafw/eventthread.h"

// Include the updated LockFreeQueue implementation here

class LockFreeQueueTest : public ::testing::Test {
public:
    static constexpr size_t QUEUE_SIZE = 1024;
    static constexpr size_t NUM_PRODUCERS = 4;
    static constexpr size_t NUM_CONSUMERS = 4;
    static constexpr size_t OPERATIONS_PER_THREAD = 100000;

    LockFreeQueue<int, QUEUE_SIZE> queue;
    std::atomic<size_t> push_count{0};
    std::atomic<size_t> pop_count{0};
    std::atomic<bool> start_flag{false};
    std::atomic<bool> stop_flag{false};

    void ProducerThread() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1, 1000);

        while (!start_flag.load(std::memory_order_acquire)) { std::this_thread::yield(); }

        for (size_t i = 0; i < OPERATIONS_PER_THREAD; ++i) {
            int value = dis(gen);
            while (!queue.push(value)) {
                std::this_thread::yield();
            }
            push_count.fetch_add(1, std::memory_order_relaxed);
        }
    }

    void ConsumerThread() {
        while (!start_flag.load(std::memory_order_acquire)) { std::this_thread::yield(); }

        while (!stop_flag.load(std::memory_order_acquire) || push_count.load(std::memory_order_acquire) > pop_count.load(std::memory_order_acquire)) {
            auto item = queue.pop();
            if (item) {
                pop_count.fetch_add(1, std::memory_order_relaxed);
            } else {
                std::this_thread::yield();
            }
        }
    }
};

TEST_F(LockFreeQueueTest, StressTest) {
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    for (size_t i = 0; i < NUM_PRODUCERS; ++i) {
        producers.emplace_back(&LockFreeQueueTest::ProducerThread, this);
    }

    for (size_t i = 0; i < NUM_CONSUMERS; ++i) {
        consumers.emplace_back(&LockFreeQueueTest::ConsumerThread, this);
    }

    start_flag.store(true, std::memory_order_release);

    auto start_time = std::chrono::high_resolution_clock::now();

    for (auto& t : producers) {
        t.join();
    }

    while (pop_count.load(std::memory_order_acquire) < NUM_PRODUCERS * OPERATIONS_PER_THREAD) {
        std::this_thread::yield();
    }

    stop_flag.store(true, std::memory_order_release);

    for (auto& t : consumers) {
        t.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    EXPECT_EQ(push_count.load(std::memory_order_relaxed), NUM_PRODUCERS * OPERATIONS_PER_THREAD);
    EXPECT_EQ(pop_count.load(std::memory_order_relaxed), NUM_PRODUCERS * OPERATIONS_PER_THREAD);

    std::cout << "Stress test completed in " << duration.count() << " ms" << std::endl;
    std::cout << "Total operations: " << push_count.load(std::memory_order_relaxed) + pop_count.load(std::memory_order_relaxed) << std::endl;
    std::cout << "Operations per second: "
              << (push_count.load(std::memory_order_relaxed) + pop_count.load(std::memory_order_relaxed)) * 1000 / duration.count() << std::endl;
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#include <gtest/gtest.h>

#include "mlafw/mlafw.h"

#include <atomic>
#include <barrier>
#include <chrono>
#include <random>
#include <variant>
#include <vector>

using namespace std::chrono_literals;

struct SomeEvent {};
struct SomeOtherEvent {};
struct BreakEventLoop {};

using mla::log::StdLogger;

class TestEventThread;

struct ThreadEvent
{
    std::variant<SomeEvent, SomeOtherEvent> event;
    TestEventThread* sender;
};

using Event = std::variant<ThreadEvent, BreakEventLoop>;

class TestEventThread : public mla::thread::EventThread<TestEventThread, Event>
{
public:
    TestEventThread(std::atomic<int>& someEventCounter,
                    std::atomic<int>& someOtherEventCounter)
        : someEventCounter(someEventCounter),
          someOtherEventCounter(someOtherEventCounter)
    {
    }

    void onEvent(const Event& event)
    {
        // Parse template event and forward event to handlers
        std::visit([this](auto&& arg) { this->onEvent(arg); }, event);
    }

    void onEvent(const ThreadEvent& event)
    {
        // Parse event from ThreadEvent and forward inner event to handlers
        std::visit([this](auto&& arg) { this->onEvent(arg); }, event.event);
    }

    void onEvent(const BreakEventLoop&)
    {
        LOG_INFO(StdLogger(), "Breaking up event loop...");
        breakEventLoop();
        LOG_INFO(StdLogger(), "Done...");
    }

    void onEvent(const SomeEvent&)
    {
        someEventCounter++;
    }

    void onEvent(const SomeOtherEvent&)
    {
        someOtherEventCounter++;
    }

    void execute() override
    {
        eventLoop();
    }

    void exit() override
    {
        breakEventLoop();
    }

    void sendEvent(TestEventThread* target,
                   std::variant<SomeEvent, SomeOtherEvent> event)
    {
        target->push(ThreadEvent{event, this});
    }

    std::atomic<int>& someEventCounter;
    std::atomic<int>& someOtherEventCounter;
};

TEST(EventThreadTest, OneThread)
{
    std::atomic<int> someEventCounter(0);
    std::atomic<int> someOtherEventCounter(0);
    TestEventThread th(someEventCounter, someOtherEventCounter);
    th.start();

    th.push(ThreadEvent{SomeEvent{}, &th});
    th.push(ThreadEvent{SomeOtherEvent{}, &th});
    th.push(BreakEventLoop{});

    th.join();

    EXPECT_EQ(someEventCounter, 1);
    EXPECT_EQ(someOtherEventCounter, 1);
}

TEST(EventThreadTest, MultiThreadStressTest)
{
    constexpr int NUM_THREADS = 50;
    constexpr int EVENTS_PER_THREAD = 1000;

    std::atomic<int> someEventCounter{0};
    std::atomic<int> someOtherEventCounter{0};
    std::atomic<int> totalPushedEvents{0};
    std::barrier sync_point(NUM_THREADS + 1); // +1 for the main thread

    std::vector<std::unique_ptr<TestEventThread>> threads;
    for(int i = 0; i < NUM_THREADS; ++i)
        threads.emplace_back(std::make_unique<TestEventThread>(
            someEventCounter, someOtherEventCounter));

    // Start all threads
    for(auto& thread : threads)
        thread->start();

    using Var = std::variant<SomeEvent, SomeOtherEvent>;

    // Lambda to generate random events
    auto generateRandomEvent = [](TestEventThread* sender) -> Event
    {
        static std::random_device rd;
        static std::mt19937 gen{rd()};
        static std::uniform_int_distribution<> dis{0, 1};
        return ThreadEvent{
            .event = dis(gen) == 0 ?
                Var(SomeEvent{}) :
                Var(SomeOtherEvent{}),
            .sender = sender
        };
    };

    // Spawn worker threads to send events
    std::vector<std::thread> workers;
    for(int i = 0; i < NUM_THREADS; ++i)
        workers.emplace_back(
            [&, i]()
            {
                sync_point.arrive_and_wait(); // Synchronize start
                for(int j = 0; j < EVENTS_PER_THREAD; ++j)
                {
                    Event event = generateRandomEvent(threads[i].get());
                    threads[j % NUM_THREADS]->push(event);
                    totalPushedEvents++;
                }
                sync_point.arrive_and_wait(); // Synchronize end
            });

    // Wait for all workers to start
    sync_point.arrive_and_wait();

    // Wait for all workers to finish
    sync_point.arrive_and_wait();

    // Wait a bit to allow event processing to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // Send BreakEventLoop to all threads
    for(auto& thread : threads)
        thread->push(BreakEventLoop{});

    // Join all threads
    for(auto& thread : threads)
        thread->join();

    // Join all workers
    for(auto& worker : workers)
        worker.join();

    // Check results
    int totalEvents = someEventCounter + someOtherEventCounter;
    EXPECT_EQ(totalEvents, NUM_THREADS * EVENTS_PER_THREAD);
    EXPECT_EQ(totalEvents, totalPushedEvents);
    LOG_INFO(StdLogger(), "Total events pushed: {}", totalPushedEvents.load());
    LOG_INFO(StdLogger(), "Total events processed: {}", totalEvents);
    LOG_INFO(StdLogger(), "SomeEvents: {}", someEventCounter.load());
    LOG_INFO(StdLogger(), "SomeOtherEvents: {}", someOtherEventCounter.load());
}

TEST(EventThreadTest, InterThreadCommunication)
{
    const int NUM_THREADS = 50;
    const int EVENTS_PER_THREAD = 1000;

    std::atomic<int> someEventCounter{0};
    std::atomic<int> someOtherEventCounter{0};
    std::atomic<int> totalPushedEvents{0};
    std::barrier sync_point(NUM_THREADS + 1); // +1 for the main thread

    std::vector<std::unique_ptr<TestEventThread>> threads;
    for(int i = 0; i < NUM_THREADS; ++i)
        threads.emplace_back(std::make_unique<TestEventThread>(
            someEventCounter, someOtherEventCounter));

    // Start all threads
    for(auto& thread : threads)
        thread->start();

    using Var = std::variant<SomeEvent, SomeOtherEvent>;

    // Lambda to generate random events
    auto generateRandomEvent = []() -> std::variant<SomeEvent, SomeOtherEvent>
    {
        static std::random_device rd;
        static std::mt19937 gen{rd()};
        static std::uniform_int_distribution<> dis{0, 1};
        return dis(gen) == 0
                   ? Var(SomeEvent{})
                   : Var(SomeOtherEvent{});
    };

    // Lambda to select a random thread
    auto selectRandomThread = [&threads](int currentIndex) -> TestEventThread*
    {
        static std::random_device rd;
        static std::mt19937 gen{rd()};
        std::uniform_int_distribution<> dis{0, NUM_THREADS - 2};
        int selectedIndex = dis(gen);
        if(selectedIndex >= currentIndex)
            selectedIndex++;
        return threads[selectedIndex].get();
    };

    // Spawn worker threads to send events
    std::vector<std::thread> workers;
    for(int i = 0; i < NUM_THREADS; ++i)
        workers.emplace_back(
            [&, i]()
            {
                sync_point.arrive_and_wait(); // Synchronize start
                for(int j = 0; j < EVENTS_PER_THREAD; ++j)
                {
                    auto event = generateRandomEvent();
                    TestEventThread* target = selectRandomThread(i);
                    threads[i]->sendEvent(target, event);
                    totalPushedEvents++;
                }
                sync_point.arrive_and_wait(); // Synchronize end
            });

    // Wait for all workers to start
    sync_point.arrive_and_wait();

    // Wait for all workers to finish
    sync_point.arrive_and_wait();

    // Wait a bit to allow event processing to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Send BreakEventLoop to all threads
    for(auto& thread : threads)
        thread->push(BreakEventLoop{});

    // Join all threads
    for(auto& thread : threads)
        thread->join();

    // Join all workers
    for(auto& worker : workers)
        worker.join();

    // Check results
    int totalEvents = someEventCounter + someOtherEventCounter;
    EXPECT_EQ(totalEvents, NUM_THREADS * EVENTS_PER_THREAD);
    EXPECT_EQ(totalEvents, totalPushedEvents);
    LOG_INFO(StdLogger(), "Total events pushed: {}", totalPushedEvents.load());
    LOG_INFO(StdLogger(), "Total events processed: {}", totalEvents);
    LOG_INFO(StdLogger(), "SomeEvents: {}", someEventCounter.load());
    LOG_INFO(StdLogger(), "SomeOtherEvents: {}", someOtherEventCounter.load());
}

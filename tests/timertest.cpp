#include <gtest/gtest.h>
#include <memory>

#include "mlafw/timer.h"
#include "mlafw/eventthread.h"

namespace mla {

struct TimerEvent {};

using GotTimer = std::variant<TimerEvent>;

class TimerTester : public timer::Receiver,
                    public thread::EventThread<TimerTester, GotTimer>,
                    public std::enable_shared_from_this<TimerTester>
{
public:
    TimerTester(int client, int interval, int count)
        : client(client), interval(std::chrono::milliseconds(interval)),
          expected_count(count), count(count)
    {
    }

    void timeout(timer::timer_id) override
    {
        thread::EventThread<TimerTester, GotTimer>::push(GotTimer{});
    }

    void onEvent(const TimerEvent&)
    {
        std::lock_guard<std::mutex> lock(mtx);
        --count;
        ++total;
        std::cout << "Client " << client << " received event. Count: " << count
                  << ", Total: " << total << "\n";
        if(count > 0)
            orderTimer();
        else
            std::cout << "Client " << client
                      << " finished. Total events: " << total << "\n";
    }

    void execute() override
    {
        std::cout << "Client " << client << " started" << "\n";
        orderTimer();
        thread::EventThread<TimerTester, GotTimer>::execute();
    }

    void orderTimer()
    {
        timer::order(this, interval);
    }

    int getTotal()
    {
        std::lock_guard<std::mutex> lock(mtx);
        return total;
    }

    bool isFinished()
    {
        std::lock_guard<std::mutex> lock(mtx);
        return count <= 0;
    }

    int getExpectedCount() const
    {
        return expected_count;
    }

    int getClient() const
    {
        return client;
    }

private:
    int client;
    std::chrono::milliseconds interval;
    std::mutex mtx;
    int expected_count;
    int count;
    int total = 0;
};

TEST(TimerTests, MultipleTimers)
{
    auto timer = timer::Timer::instance();
    timer->start();

    const std::vector<std::shared_ptr<TimerTester>> clients = {
        std::make_shared<TimerTester>(1, 1, 1000),
        std::make_shared<TimerTester>(2, 50, 40),
        std::make_shared<TimerTester>(3, 100, 10),
        std::make_shared<TimerTester>(4, 300, 10),
        std::make_shared<TimerTester>(5, 1000, 4),
        std::make_shared<TimerTester>(6, 500, 10)};

    for(auto& client : clients)
        client->start();

    // Wait for all clients to finish or timeout after 10 seconds
    auto start = std::chrono::steady_clock::now();
    while(std::chrono::steady_clock::now() - start < std::chrono::seconds(10))
    {
        bool all_finished = true;
        for(const auto& client : clients)
        {
            if(!client->isFinished())
            {
                all_finished = false;
                break;
            }
        }
        if(all_finished)
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }

    // Print results and perform assertions
    bool all_timers_handled = true;
    for(const auto& client : clients)
    {
        int actual_count = client->getTotal();
        int expected_count = client->getExpectedCount();
        std::cout << "Client " << client->getClient() << ": " << actual_count
                  << " / " << expected_count << " events" << "\n";

        EXPECT_TRUE(client->isFinished())
            << "Client " << client->getClient() << " did not finish in time";

        if(actual_count != expected_count)
        {
            all_timers_handled = false;
            std::cout << "Error: Client " << client->getClient() << " handled "
                      << actual_count << " events, expected " << expected_count
                      << "\n";
        }
    }

    EXPECT_TRUE(all_timers_handled) << "Not all timers were handled correctly";

    for(auto& client : clients)
    {
        client->exit();
        client->join();
    }

    timer->exit();
    timer->join();
}

TEST(TimerTests, SingleTimer)
{
    auto timer = timer::Timer::instance();
    timer->start();
    auto client1 = std::make_shared<TimerTester>(1, 500, 1000);
    client1->start();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    client1->exit();
    client1->join();
    timer->exit();
    timer->join();
}

TEST(TimerTests, MultipleTimersSingleThread)
{
    auto timer = timer::Timer::instance();
    timer->start();

    std::vector<std::shared_ptr<TimerTester>> clients = {
        std::make_shared<TimerTester>(1, 1, 1000),
        std::make_shared<TimerTester>(2, 50, 40),
        std::make_shared<TimerTester>(3, 100, 10),
        std::make_shared<TimerTester>(4, 300, 10),
        std::make_shared<TimerTester>(5, 1000, 4),
        std::make_shared<TimerTester>(6, 500, 10)};

    for(auto& client : clients)
        client->start();

    std::this_thread::sleep_for(std::chrono::seconds(2));

    for(auto& client : clients)
    {
        client->exit();
        client->join();
    }

    timer->exit();
    timer->join();
}

} // namespace mla

#include <gtest/gtest.h>
#include <memory>

#include "mlafw/mla.h"
#include "mlafw/mlaeventthread.h"
#include "mlafw/mlalog.h"
#include "mlafw/mlatimer.h"

namespace mla {

struct TimerEvent {};
using GotTimer = std::variant<TimerEvent>;

struct TimerTester :
    public timer::Receiver,
    public thread::EventThread<TimerTester, GotTimer>,
    public std::enable_shared_from_this<TimerTester>
{
    TimerTester(int client, int interval = 1, int count = 100) :
        client(client), interval(interval), count(count)
    {
    }

    void timeout(timer::timer_id) override
    {
        thread::EventThread<TimerTester, GotTimer>::push(GotTimer{});
    }

    void onEvent(const TimerEvent&)
    {
        --count;
        ++total;
        LOG_INFO(LOG, "Recv event: " << client << " count: " << total);
        if (count > 0)
            orderTimer();
        else
            LOG_INFO(LOG, "Timer total: " << total << " client: " << client);
    }

    void execute() override
    {
        LOG_INFO(LOG, "Client started: " << client);
        orderTimer();
        thread::EventThread<TimerTester, GotTimer>::execute();
    }

    void orderTimer()
    {
        timer::order(this, interval);
    }

    log::StdLogger LOG = log::StdLogger("TimerServiceTests");

    int client = 0;
    std::chrono::milliseconds interval;
    int count = 0;
    int total = 0;
};

TEST(TimerTests, SingleTimer)
{
    auto timer = timer::MlaTimer::instance();
    timer->start();
    auto client1 = std::make_shared<TimerTester>(1,500,1000);
    client1->start();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    client1->exit();
    client1->join();
    timer->exit();
    timer->join();
}

TEST(TimerTests, MultipleTimers)
{
    auto timer = timer::MlaTimer::instance();
    timer->start();

    std::vector<std::shared_ptr<TimerTester>> clients = {
        std::make_shared<TimerTester>(1, 1, 1000),
        std::make_shared<TimerTester>(2, 50, 40),
        std::make_shared<TimerTester>(3, 100, 10),
        std::make_shared<TimerTester>(4, 300, 10),
        std::make_shared<TimerTester>(5, 1000, 4),
        std::make_shared<TimerTester>(6, 500, 10)
    };

    for(auto& client : clients)
        client->start();

    std::this_thread::sleep_for(std::chrono::seconds(2));

    for(auto& client : clients)
    {
        client->exit(); client->join();
    }

    timer->exit();
    timer->join();
}

}


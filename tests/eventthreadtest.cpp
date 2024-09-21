#include <gtest/gtest.h>
#include "mlafw/mlafw.h"

#include <variant>

using namespace std::chrono_literals;

struct SomeEvent {};
struct SomeOtherEvent {};
struct BreakEventLoop {};
using mla::log::StdLogger;

using Event = std::variant<SomeEvent, SomeOtherEvent, BreakEventLoop>;

struct TestEventThread :
    public mla::thread::EventThread<TestEventThread, Event>
{
    auto onEvent(const BreakEventLoop&) -> void
    {
        LOG_INFO(StdLogger(), "Breaking up event loop in 0.5 secs...");
        usleep(500000);
        breakEventLoop();
        LOG_INFO(StdLogger(), "Done...");
    }

    auto onEvent(const SomeEvent&) -> void
    {
        LOG_INFO(StdLogger(), "Hola from SomeEvent handler!");
        usleep(400000);
        someEventHandled = true;
    }

    auto onEvent(const SomeOtherEvent&) -> void
    {
        LOG_INFO(StdLogger(), "Hola from SomeOtherEvent handler!");
        usleep(200000);
        someOtherEventHandled = true;
    }

    virtual auto execute() -> void override
    {
        eventLoop();
    }

    virtual auto exit() -> void override
    {
        breakEventLoop();
    }

    bool someEventHandled = false;
    bool someOtherEventHandled = false;
};

TEST(EventThreadTest, OneThread)
{
    TestEventThread th{};
    th.start();

    th.push(SomeEvent{});
    th.push(SomeOtherEvent{});
    th.push(BreakEventLoop{});
    th.join();

    EXPECT_TRUE(th.someEventHandled);
    EXPECT_TRUE(th.someOtherEventHandled);
}


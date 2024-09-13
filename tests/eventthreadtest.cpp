#include <gtest/gtest.h>
#include "mlafw/mla.h"

#include <chrono>
#include <iostream>
#include <thread>
#include <variant>

using namespace std::chrono_literals;

namespace evt = mla::event;

struct Event : mla::event::EventBase {};
struct SomeEvent : Event {};
struct SomeOtherEvent : Event {};
struct BreakEventLoop : Event {};
using mla::log::StdLogger;

struct EventThread :
    public mla::thread::Thread,
    evt::Handler<SomeEvent>,
    evt::Handler<SomeOtherEvent>,
    evt::Handler<BreakEventLoop>
{
    auto onEvent(const BreakEventLoop&) -> void override
    {
        LOG_INFO(StdLogger(), "Breaking up event loop in 0.5 secs...");
        usleep(500000);
        breakEventLoop();
        LOG_INFO(StdLogger(), "Done...");
    }

    auto onEvent(const SomeEvent&) -> void override
    {
        LOG_INFO(StdLogger(), "Hola from SomeEvent handler!");
        usleep(400000);
        someEventHandled = true;
    }

    auto onEvent(const SomeOtherEvent&) -> void override
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
    EventThread th;
    th.start();

    th.push(std::make_shared<SomeEvent>());
    th.push(std::make_shared<SomeOtherEvent>());
    th.push(std::make_shared<BreakEventLoop>());
    th.join();

    EXPECT_TRUE(th.someEventHandled);
    EXPECT_TRUE(th.someOtherEventHandled);
}

using SomeVariant = std::variant<
    SomeEvent,
    SomeOtherEvent,
    BreakEventLoop>;

struct VariantEventThread :
    public mla::thread::Thread,
    evt::BlockingVariantQueue<VariantEventThread, SomeVariant>
{
    virtual auto execute() -> void override { eventLoop(); }

    virtual auto exit() -> void override
    {
        breakEventLoop();
    }

    auto operator()(const SomeEvent&) -> void
    {
        LOG_INFO(StdLogger(), "Hola from SomeEvent handler!");
        std::this_thread::sleep_for(0.5s);
    }

    auto operator()(const SomeOtherEvent&) -> void
    {
        LOG_INFO(StdLogger(), "Hola from SomeOtherEvent handler!");
        std::this_thread::sleep_for(0.5s);
    }

    auto operator()(const BreakEventLoop&) -> void
    {
        LOG_INFO(StdLogger(), "Breaking up event loop in 0.5 secs...");
        std::this_thread::sleep_for(0.5s);
        breakEventLoop();
        LOG_INFO(StdLogger(), "Done...");
    }
};

TEST(EventThreadTest, OneThreadVariant)
{
    VariantEventThread th;
    th.start();

    th.push(SomeEvent {});
    th.push(SomeOtherEvent {});
    th.push(BreakEventLoop {});
    th.join();
}


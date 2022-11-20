#include <gtest/gtest.h>

#include "mlafw/mla.h"

struct A : mla::event::EventBase {};
struct B : A {};
struct C : B {};

struct Handler :
    mla::event::Handler<A>,
    mla::event::Handler<B>,
    mla::event::Handler<C>
{
    void onEvent(const A&) override { aHandled = true; }
    void onEvent(const B&) override { bHandled = true; }
    void onEvent(const C&) override { cHandled = true; }

    bool aHandled = false;
    bool bHandled = false;
    bool cHandled = false;

    auto reset() -> void { aHandled = bHandled = cHandled = false; }
};

TEST(EventTestSuite, DispatchTest)
{
    // Test events call correct handlers and do not touch wrong handlers

    Handler handler;

    handler.processEvent(std::make_shared<A>());
    EXPECT_TRUE(handler.aHandled && !handler.bHandled && !handler.cHandled);

    handler.reset();

    handler.processEvent(std::make_shared<B>());
    EXPECT_TRUE(!handler.aHandled && handler.bHandled && !handler.cHandled);

    handler.reset();

    handler.processEvent(std::make_shared<C>());
    EXPECT_TRUE(!handler.aHandled && !handler.bHandled && handler.cHandled);
}

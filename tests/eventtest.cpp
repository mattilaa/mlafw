#include <gtest/gtest.h>

#include "mlafw/mla.h"

struct A : mla::event::EventBase {};
struct B : A {};
struct C : B {};

struct Handler : mla::event::Sub<A>, mla::event::Sub<B>, mla::event::Sub<C> {
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

    handler.dispatch(A{});
    EXPECT_TRUE(handler.aHandled && !handler.bHandled && !handler.cHandled);

    handler.reset();

    handler.dispatch(B{});
    EXPECT_TRUE(!handler.aHandled && handler.bHandled && !handler.cHandled);

    handler.reset();

    handler.dispatch(C{});
    EXPECT_TRUE(!handler.aHandled && !handler.bHandled && handler.cHandled);
}

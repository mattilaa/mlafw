#include <gtest/gtest.h>

#include "mlafw/mla.h"

struct LogThread : mla::thread::Thread
{
    auto execute() -> void override
    {
        for(int i = 0; i < 100; ++i)
        {
            std::stringstream ss;
            ss << "Log entry no: " << i << ", hello from thread id: " << getId();
            LOG(log, "Hello! " << ss.str());
        }
    }

    auto exit() -> void override
    {
    }

    mla::log::StdOutLogger log{"LogThread", true};
};

TEST(LogTests, InfoLogTest)
{
    mla::log::StdOutLogger log {"TestLog"};
    std::stringstream ss;
    ss << "Hello" << ", world!";
    log.log(ss.str().c_str());
}

TEST(LogTests, InfoLogTestColor)
{
    mla::log::StdOutLogger log {"TestLog", true};
    std::stringstream ss;
    ss << "Hello" << ", world!";
    log.log(ss.str());
}

TEST(LogTests, ThreadTest)
{
    LogThread t1, t2, t3, t4;

    t1.start();
    t2.start();
    t3.start();
    t4.start();

    t1.join();
    t2.join();
    t3.join();
    t4.join();
}

TEST(LogTests, ContextTest)
{
    mla::log::StdOutLogger log1 {"MyLog"};
    mla::log::StdOutLogger log5 {log1, "SubLog"};
    LOG(log5, "Hey");
}


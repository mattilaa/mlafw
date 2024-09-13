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
            LOG_INFO(log, "str: " << ss.str());
            LOG_INFO(log, "int " << 6);
        }
    }

    auto exit() -> void override
    {
    }

    mla::log::StdLogger log{"LogThread", true};
};

TEST(LogTests, InfoLogTest)
{
    mla::log::StdLogger log {"TestLog"};
    std::stringstream ss;
    ss << "Hello" << ", world!";
    log.log(mla::log::LogLevel::INFO, ss.str().c_str());
}

TEST(LogTests, InfoLogTestColor)
{
    mla::log::StdLogger log {"TestLog", true};
    std::stringstream ss;
    ss << "Hello" << ", world!";
    log.log(mla::log::LogLevel::INFO, ss.str().c_str());
}

TEST(LogTests, LogLevelTests)
{
    mla::log::StdLogger log {"TestLog", true};
    LOG_INFO(log, "Hello I'm LOG_INFO: " << 2.3 << ", 😍");
    LOG_WARNING(log, "Hello I'm LOG_WARNING: " << true<< ", 😱");
    LOG_ERROR(log, "Hello I'm LOG_ERROR: 😭");
    LOG_DEBUG(log, "Hello I'm LOG_DEBUG: 🦗");
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
    mla::log::StdLogger log1 {"MyLog"};
    mla::log::StdLogger log5 {log1, "SubLog"};
    LOG_INFO(log5, "Hey");
}


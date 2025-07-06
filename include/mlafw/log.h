#ifndef __MLA_LOG_H__
#define __MLA_LOG_H__

#include <chrono>
#include <format>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string_view>

namespace mla::log
{

enum class LogLevel
{
    INFO,
    WARNING,
    ERROR,
    DEBUG
};

static std::ostream& operator<<(std::ostream& oss, LogLevel level)
{
    switch(level)
    {
        case LogLevel::INFO:
            return oss << "INFO";
        case LogLevel::WARNING:
            return oss << "WARNING";
        case LogLevel::ERROR:
            return oss << "ERROR";
        case LogLevel::DEBUG:
            return oss << "DEBUG";
    }
    return oss; // Because of some stupid compilers
}

static std::string toColor(LogLevel level)
{
    switch(level)
    {
        case LogLevel::INFO:
            return "\x1b[32;1m";
        case LogLevel::WARNING:
            return "\x1b[33;1m";
        case LogLevel::ERROR:
            return "\x1b[31;1m";
        case LogLevel::DEBUG:
            return "\x1b[36;1m";
    }
    return ""; // Because of some stupid compilers
}

class StdLogger
{
public:
    StdLogger(std::string_view ctx = "", bool use_color = false)
        : ctx(ctx), use_color(use_color)
    {
    }

    StdLogger(const StdLogger &log, std::string_view new_ctx)
        : ctx(log.ctx.empty() ? std::string(new_ctx)
                              : log.ctx + "/" + std::string(new_ctx)),
          use_color(log.use_color)
    {
    }

    static std::string timestamp()
    {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::floor<std::chrono::microseconds>(now);
        auto ms = std::chrono::duration_cast<std::chrono::microseconds>(
                      time.time_since_epoch())
                      .count() %
                  1000000;

        std::time_t tt = std::chrono::system_clock::to_time_t(now);
        std::tm tm;
#ifdef _WIN32
        localtime_s(&tm, &tt);
#else
        localtime_r(&tt, &tm);
#endif

        return std::format("{:02d}:{:02d}:{:02d}.{:06d}", 
                          tm.tm_hour, tm.tm_min, tm.tm_sec, ms);
    }

    void log(LogLevel level, std::string_view msg)
    {
        std::string formatted = std::format("{}{}{} {} {}{}\n",
            use_color ? toColor(level) : "",
            static_cast<int>(level) == 0 ? "INFO" : 
            static_cast<int>(level) == 1 ? "WARNING" :
            static_cast<int>(level) == 2 ? "ERROR" : "DEBUG",
            use_color ? "\x1b[0m" : "",
            timestamp(),
            ctx.empty() ? std::string(msg) : ctx + " " + std::string(msg),
            use_color ? "\x1b[0m" : "");

        static std::mutex log_mutex;
        std::lock_guard<std::mutex> lock(log_mutex);
        
        if(level == LogLevel::INFO || level == LogLevel::DEBUG)
            std::cout << formatted;
        else
            std::cerr << formatted;
    }

private:
    std::string ctx;
    bool use_color;
};

}  // namespace mla::log

#define LOG_INFO(logger, ...)                                \
    do                                                       \
    {                                                        \
        logger.log(mla::log::LogLevel::INFO, std::format(__VA_ARGS__)); \
    } while(0)

#define LOG_DEBUG(logger, ...)                               \
    do                                                       \
    {                                                        \
        logger.log(mla::log::LogLevel::DEBUG, std::format(__VA_ARGS__)); \
    } while(0)

#define LOG_WARNING(logger, ...)                             \
    do                                                       \
    {                                                        \
        logger.log(mla::log::LogLevel::WARNING, std::format(__VA_ARGS__)); \
    } while(0)

#define LOG_ERROR(logger, ...)                               \
    do                                                       \
    {                                                        \
        logger.log(mla::log::LogLevel::ERROR, std::format(__VA_ARGS__)); \
    } while(0)

#endif  // __MLA_LOG_H__

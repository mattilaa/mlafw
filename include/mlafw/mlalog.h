#ifndef __MLA_LOG_H__
#define __MLA_LOG_H__

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
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

class Logger
{
public:
    virtual ~Logger() = default;

    virtual void log(LogLevel level, const std::string& msg) = 0;

    static std::string timestamp()
    {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::floor<std::chrono::microseconds>(now);
        auto ms = std::chrono::duration_cast<std::chrono::microseconds>(
                    time.time_since_epoch())
                    .count() %
                  1000000;

        std::time_t tt = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&tt);

        std::ostringstream oss;
        oss << std::put_time(&tm, "%H:%M:%S") << '.' << std::setfill('0')
            << std::setw(6) << ms;
        return oss.str();
    }
};

class StdLogger : public Logger
{
public:
    StdLogger(std::string_view ctx = "", bool use_color = false)
      : ctx(ctx), use_color(use_color)
    {
    }

    StdLogger(const StdLogger& log, std::string_view new_ctx)
      : ctx(log.ctx.empty() ? std::string(new_ctx)
                            : log.ctx + "/" + std::string(new_ctx)),
        use_color(log.use_color)
    {
    }

    void log(LogLevel level, const std::string& msg) override
    {
        std::string level_str;
        std::string color_code;

        switch(level)
        {
            case LogLevel::INFO:
                level_str = "INFO";
                color_code = "\x1b[32;1m";
                break;
            case LogLevel::WARNING:
                level_str = "WARNING";
                color_code = "\x1b[33;1m";
                break;
            case LogLevel::ERROR:
                level_str = "ERROR";
                color_code = "\x1b[31;1m";
                break;
            case LogLevel::DEBUG:
                level_str = "DEBUG";
                color_code = "\x1b[36;1m";
                break;
        }

        std::ostringstream ss;
        ss << (use_color ? color_code : "") << level_str
           << (use_color ? "\x1b[0m" : "") << " " << timestamp() << " "
           << (ctx.empty() ? "" : ctx + " ") << msg
           << (use_color ? "\x1b[0m" : "") << std::endl;

        // Using a single cout call to ensure atomicity
        std::cout << ss.str();
    }

private:
    std::string ctx;
    bool use_color;
};

}  // namespace mla::log

#define LOG_INFO(logger, ...)                            \
    do                                                   \
    {                                                    \
        std::ostringstream ss_;                          \
        ss_ << __VA_ARGS__;                              \
        logger.log(mla::log::LogLevel::INFO, ss_.str()); \
    } while(0)

#define LOG_DEBUG(logger, ...)                            \
    do                                                    \
    {                                                     \
        std::ostringstream ss_;                           \
        ss_ << __VA_ARGS__;                               \
        logger.log(mla::log::LogLevel::DEBUG, ss_.str()); \
    } while(0)

#define LOG_WARNING(logger, ...)                            \
    do                                                      \
    {                                                       \
        std::ostringstream ss_;                             \
        ss_ << __VA_ARGS__;                                 \
        logger.log(mla::log::LogLevel::WARNING, ss_.str()); \
    } while(0)

#define LOG_ERROR(logger, ...)                            \
    do                                                    \
    {                                                     \
        std::ostringstream ss_;                           \
        ss_ << __VA_ARGS__;                               \
        logger.log(mla::log::LogLevel::ERROR, ss_.str()); \
    } while(0)

#endif  // __MLA_LOG_H__

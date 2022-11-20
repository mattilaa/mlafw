#ifndef __MLA_LOG_H__
#define __MLA_LOG_H__

#include <chrono>
#include <iostream>
#include <sstream>

#define LOG(l, x)                                                              \
    {                                                                          \
        std::stringstream ___ss;                                               \
        ___ss << x;                                                            \
        l.log(___ss.str().c_str());                                            \
    }

namespace mla::log {

class Logger
{
    static constexpr unsigned kBufLength = 13;
public:
    virtual ~Logger() = default;

    virtual auto log(const std::string& msg) -> void = 0;

    virtual auto log(const char* msg) -> void = 0;

    static auto timestamp() -> std::string
    {
        namespace c = std::chrono;
        c::system_clock::time_point tp = c::system_clock::now();
        std::time_t tt = c::system_clock::to_time_t(tp);
        std::tm gmt;
        gmtime_r(&tt, &gmt);
        c::duration<double> frac = (tp - c::system_clock::from_time_t(tt))
                                   + c::seconds(gmt.tm_sec);
        std::string buffer {"", kBufLength};
        sprintf(&buffer.front(),
                "%02d:%02d:%09.6f",
                gmt.tm_hour,
                gmt.tm_min,
                frac.count());
        return buffer;
    }
};

class StdOutLogger : public Logger
{
    std::string ctx;
    bool isColor;
public:
    StdOutLogger(const char* ctx = "", bool isColor = false) :
        ctx(ctx), isColor(isColor)
    {}

    StdOutLogger(const StdOutLogger& log, const char* ctx) :
        ctx(std::string(log.ctx + "/") + ctx),
        isColor(log.isColor)
    {}

    auto log(const std::string& msg) -> void override
    {
        log(msg.c_str());
    }

    auto log(const char* msg) -> void override
    {
        std::stringstream ss;
        if(isColor)
            ss << "\x1b[32;1m[INFO]\x1b[0m ";
        else
            ss << "[INFO] ";
        ss << timestamp() << " " << ctx << (ctx.empty() ? "" : " ") << msg
           << "\n";
        std::cout << ss.str();
    }
};

} // namespace mla::log

#endif

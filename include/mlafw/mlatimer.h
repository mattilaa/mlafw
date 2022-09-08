#ifndef __MLA_TIMER_H__
#define __MLA_TIMER_H__

#include "mladefs.h"
#include "mlacommon.h"

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>

namespace mla::timer {

using clock_type = std::chrono::high_resolution_clock;
using duration = std::uint64_t;
using timer_id = unsigned;

struct Receiver
{
    virtual ~Receiver() = default;

    virtual auto timeout(timer_id id) -> void = 0;
};

#ifdef USE_SMART_POINTER_RECEIVER
using receiver_type = std::weak_ptr<Receiver>;
#else
using receiver_type = Receiver*;
#endif

struct Order
{
    receiver_type rcv;
    clock_type::time_point last;
    timer_id id;

    auto operator<(const Order& rhs) const -> bool { return rhs.last < last; }
};

class MlaTimer
{
    std::priority_queue<Order> _queue;
    std::mutex _mutex;
    std::condition_variable _cv;
    bool _running = false;

public:
    [[nodiscard]] static auto instance() -> MlaTimer*
    {
        static MlaTimer instance;
        return &instance;
    }

    auto run() -> void
    {
        while(true)
        {
            std::unique_lock<std::mutex> lock {_mutex};

            while(_queue.empty() && _running)
                _cv.wait(lock);

            if(__MLA_UNLIKELY(!_running))
                break;

            const auto timer = _queue.top();
            const auto now = clock_type::now();
            const auto expire = timer.last;

            bool handled = false;

            if(__MLA_LIKELY(expire > now))
            {
                const auto timeout = expire - now;
                std::cv_status s = _cv.wait_for(lock, timeout);
                handled = s == std::cv_status::timeout;
            }
            else
            {
                handled = true;
            }

            if(handled)
            {
                _queue.pop();

#ifdef USE_SMART_POINTER_RECEIVER
                if(auto ptr = timer.rcv.lock())
#else
                if(auto ptr = timer.rcv)
#endif
                {
                    auto t {timer.id};
                    ptr->timeout(t);
                }
            }
        }
    }

    auto exit() -> void
    {
        _running = false;
        _cv.notify_one();
    }

    auto order(receiver_type cb, std::uint64_t duration) -> unsigned
    {
        std::lock_guard<std::mutex> lock {_mutex};

        clock_type::time_point last {
            clock_type::now() + std::chrono::milliseconds(duration)
        };
        auto id = mla::Id::getId();
        _queue.emplace(Order {cb, last, id});
        _cv.notify_one();

        return id;
    }

    /*
    auto cancel(unsigned id, bool handle_timeout = false)
    {
        // \todo implement
        std::lock_guard<std::mutex> lock {_mutex};
    }
    */
};

inline auto order(receiver_type cb, std::uint64_t duration) -> unsigned
{
    return MlaTimer::instance()->order(cb, duration);
}

} // namespace mla::timer

#endif

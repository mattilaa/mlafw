#ifndef __MLA_TIMER_H__
#define __MLA_TIMER_H__

#include "mlacommon.h"
#include "mlafw/mlaeventthread.h"
#include "mlafw/mlalog.h"
#include "mlafw/mlathread.h"

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <vector>
#include <atomic>
#include <unordered_set>

namespace mla::timer {

using clock_type = std::chrono::steady_clock;
using duration = std::chrono::milliseconds;
using timer_id = unsigned;

class Receiver
{
public:
    virtual ~Receiver() = default;
    virtual void timeout(timer_id id) = 0;
};

#ifdef USE_SMART_POINTER_RECEIVER
using receiver_type = std::weak_ptr<Receiver>;
#else
using receiver_type = Receiver*;
#endif

struct Order
{
    receiver_type rcv;
    clock_type::time_point expiry;
    timer_id id;

    bool operator>(const Order& rhs) const { return expiry > rhs.expiry; }
};

class MlaTimer : public thread::Thread
{
public:
    static MlaTimer* instance()
    {
        static MlaTimer instance;
        return &instance;
    }

    ~MlaTimer() = default;

    void execute() override
    {
        [[likely]] while(_running)
        {
            std::unique_lock<std::mutex> lock{_mutex};

            if(_queue.empty())
            {
                _cv.wait(lock, [this] { return !_running || !_queue.empty(); });
                [[unlikely]] if(!_running)
                    break;
            }

            auto now = clock_type::now();
            if(_queue.top().expiry > now)
            {
                _cv.wait_until(lock, _queue.top().expiry);
                continue;
            }

            auto order = std::move(const_cast<Order&>(_queue.top()));
            _queue.pop();

            if(_cancelled_timers.find(order.id) == _cancelled_timers.end())
            {
#ifdef USE_SMART_POINTER_RECEIVER
                if(auto receiver = order.rcv.lock())
#else
                if(auto receiver = order.rcv)
#endif
                {
                    receiver->timeout(order.id);
                }
            }
            else
            {
                _cancelled_timers.erase(order.id);
            }
        }
    }

    void exit() override
    {
        _running = false;
        _cv.notify_one();
    }

    timer_id order(receiver_type cb, duration timeout_duration)
    {
        std::lock_guard<std::mutex> lock{_mutex};

        auto expiry = clock_type::now() + timeout_duration;
        auto id = mla::Id::getId();
        _queue.emplace(Order{std::move(cb), expiry, id});
        _cv.notify_one();

        return id;
    }

    bool cancel(timer_id id)
    {
        std::lock_guard<std::mutex> lock{_mutex};
        return _cancelled_timers.insert(id).second;
    }

private:
    MlaTimer() = default;
    MlaTimer(const MlaTimer&) = delete;
    MlaTimer& operator=(const MlaTimer&) = delete;

    std::priority_queue<Order, std::vector<Order>, std::greater<>> _queue;
    std::unordered_set<timer_id> _cancelled_timers;
    std::mutex _mutex;
    std::condition_variable _cv;
    std::atomic_bool _running{true};
};

inline timer_id order(receiver_type cb, duration timeout_duration)
{
    return MlaTimer::instance()->order(std::move(cb), timeout_duration);
}

inline bool cancel(timer_id id)
{
    return MlaTimer::instance()->cancel(id);
}

} // namespace mla::timer

#endif

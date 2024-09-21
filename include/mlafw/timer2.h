#ifndef __MLA_TIMER2_H__
#define __MLA_TIMER2_H__

#include "mlafw/common.h"
#include "mlafw/thread.h"

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <vector>
#include <atomic>

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

class Timer : public thread::Thread
{
    struct TimerEvent
    {
        std::chrono::steady_clock::time_point expiry;
        receiver_type receiver;
        timer_id id;

        bool operator>(const TimerEvent& other) const
        {
            return expiry > other.expiry;
        }
    };

public:
    static Timer* instance()
    {
        static Timer instance;
        return &instance;
    }

    ~Timer() = default;

    void execute() override
    {
        [[likely]] while(_running)
        {
            std::unique_lock<std::mutex> lock(_mutex);

            if(_queue.empty())
            {
                _cv.wait(lock,
                        [this] { return !_queue.empty() || !_running; });
                [[unlikely]] if(!_running)
                    break;
            }

            auto now = std::chrono::steady_clock::now();
            if(_queue.top().expiry <= now)
            {
                auto event = _queue.top();
                _queue.pop();
                lock.unlock();

                event.receiver->timeout(event.id);
            }
            else
            {
                _cv.wait_until(lock, _queue.top().expiry);
            }
        }
    }

    void exit() override
    {
        _running = false;
        _cv.notify_all();
    }

    timer_id order(receiver_type cb, duration timeout)
    {
        auto now = std::chrono::steady_clock::now();
        auto id = mla::Id::getId();

        {
            std::lock_guard<std::mutex> lock(_mutex);
            _queue.push({.expiry = now + timeout, .receiver = cb, .id = id});
        }

        _cv.notify_one();
        return id;
    }

    bool cancel(timer_id id)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        bool found = false;

        // Create a temporary vector to hold all events
        std::vector<TimerEvent> temp;

        // Move all events except the one to be cancelled to the temporary
        // vector
        while(!_queue.empty())
        {
            if(_queue.top().id == id)
            {
                found = true;
                _queue.pop();
            }
            else
            {
                temp.push_back(_queue.top());
                _queue.pop();
            }
        }

        // Push all events back into the priority queue
        for(const auto& event : temp)
        {
            _queue.push(event);
        }

        return found;
    }

private:
    Timer() = default;
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    std::priority_queue<TimerEvent, std::vector<TimerEvent>, std::greater<>> _queue;
    std::mutex _mutex;
    std::condition_variable _cv;
    std::atomic_bool _running{true};
};

inline timer_id order(receiver_type cb, duration timeout)
{
    return Timer::instance()->order(std::move(cb), timeout);
}

inline bool cancel(timer_id id)
{
    return Timer::instance()->cancel(id);
}

} // namespace mla::timer

#endif


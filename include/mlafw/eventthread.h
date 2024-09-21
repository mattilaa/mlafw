#ifndef __MLA_EVENTTHREAD_H__
#define __MLA_EVENTTHREAD_H__

#include "thread.h"

#include "blockingconcurrentqueue.h"

#include <variant>

namespace mla::thread {

static constexpr int kDefaultQueueSize = 10000;

template<typename EventType>
class BlockingEventQueue
{
public:
    virtual ~BlockingEventQueue() = default;

    virtual void processEvent(const EventType& event) = 0;

    void push(const EventType& event);

    void push(EventType&& event);

    auto isLockFree() -> bool { return _queue.is_lock_free(); }

    void eventLoop();

    void breakEventLoop();

protected:
    moodycamel::BlockingConcurrentQueue<
        EventType,
        moodycamel::ConcurrentQueueDefaultTraits> _queue {kDefaultQueueSize};

    std::atomic_bool _isRunning{false};
};

template <typename Owner, typename EventType>
class EventThread : public mla::thread::Thread,
                    public BlockingEventQueue<EventType>
{
public:
    virtual ~EventThread() = default;

    void execute() override
    {
        BlockingEventQueue<EventType>::eventLoop();
    }

    void exit() override
    {
        BlockingEventQueue<EventType>::breakEventLoop();
    }

    void processEvent(const EventType& event) override;
};

// BlockingEventQueue implementation
template<typename EventType>
void BlockingEventQueue<EventType>::push(const EventType& event)
{
    _queue.enqueue(event);
}

template<typename EventType>
void BlockingEventQueue<EventType>::push(EventType&& event)
{
    _queue.enqueue(std::move(event));
}

template<typename EventType>
void BlockingEventQueue<EventType>::eventLoop()
{
    _isRunning.store(true);

    while(true)
    {
        EventType evt;
        _queue.wait_dequeue(evt);
        processEvent(evt);

        [[unlikely]] if(!_isRunning.load())
            break;
    }
}

template<typename EventType>
void BlockingEventQueue<EventType>::breakEventLoop()
{
    _isRunning.store(false);

    // Enqueue a dump object just to make event loop exit.
    // Queue does not support notify.. Maybe fix this later
    _queue.enqueue(EventType{});
}

template<typename Owner, typename EventType>
void EventThread<Owner, EventType>::processEvent(const EventType& event)
{
    auto* owner = static_cast<Owner*>(this);
    std::visit([&](const auto& e) { owner->onEvent(e); }, event);
}

} // namespace mla::thread

#endif

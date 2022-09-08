#ifndef __MLA_EVENTTHREAD_H__
#define __MLA_EVENTTHREAD_H__

#include "mladefs.h"
#include "mlathread.h"

#include "concurrentqueue/blockingconcurrentqueue.h"

namespace mla::thread {

static constexpr int kDefaultQueueSize = 10000;

template<typename EventType, typename VisitorType = void>
class BlockingEventQueue
{
public:
    virtual ~BlockingEventQueue() = default;

    virtual void processEvent(const EventType& event) = 0;

    void push(EventType& event);

    void push(EventType&& event);

    bool isLockFree() const { return _q.is_lock_free(); }

    void eventLoop();

    void breakEventLoop();

protected:
    moodycamel::BlockingConcurrentQueue<
        EventType,
        moodycamel::ConcurrentQueueDefaultTraits> _q {kDefaultQueueSize};

    std::atomic_bool _isRunning = ATOMIC_VAR_INIT(false);
};

template<typename EventType, typename VisitorType>
class MlaEventThread :
    public mla::thread::Thread,
    public BlockingEventQueue<EventType>
{
public:
    virtual ~MlaEventThread() = default;

    virtual auto execute() -> void override
    {
        BlockingEventQueue<EventType>::eventLoop();
    }

    virtual void processEvent(const EventType& event) override
    {
    }

    virtual auto exit() -> void override
    {
        BlockingEventQueue<EventType>::breakEventLoop();
    }

    VisitorType visitor;
};

// BlockingEventQueue implementation
template<typename EventType, typename VisitorType>
void BlockingEventQueue<EventType, VisitorType>::push(EventType& event)
{
    /// \todo remove
    push(std::move(event));
}

template<typename EventType, typename VisitorType>
void BlockingEventQueue<EventType, VisitorType>::push(EventType&& event)
{
    _q.enqueue(std::move(event));
}

template<typename EventType, typename VisitorType>
void BlockingEventQueue<EventType, VisitorType>::eventLoop()
{
    _isRunning.store(true);

    while(true)
    {
        EventType evt;
        _q.wait_dequeue(evt);
        processEvent(evt);

        if(__MLA_UNLIKELY(!_isRunning.load()))
            break;
    }
}

template<typename EventType, typename VisitorType>
void BlockingEventQueue<EventType, VisitorType>::breakEventLoop()
{
    _isRunning.store(false);

    // Enqueue a dump object just to make event loop exit.
    _q.enqueue(EventType{});
}

} // namespace mla::thread

#endif

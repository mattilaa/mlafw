#ifndef __MLA_EVENT_H__
#define __MLA_EVENT_H__

#include "mlafw/detail/mlaeventdetail.h"

#include <variant>

namespace mla::event {

template<typename EventType>
class BlockingEventQueue
{
public:
    virtual ~BlockingEventQueue() = default;

    virtual auto processEvent(const EventType& evt) -> void = 0;

    void push(EventType&& evt)
    {
        _q.enqueue(std::move(evt));
    }

    bool isLockFree() const { return _q.is_lock_free(); }

    void eventLoop()
    {
        _isRunning.store(true);

        while (true)
        {
            EventType evt;
            _q.wait_dequeue(evt);
            processEvent(evt);

            if (__MLA_UNLIKELY(!_isRunning.load()))
                break;
        }
    }

    void breakEventLoop()
    {
        _isRunning.store(false);

        // Enqueue a dump object just to make event loop exit.
        // \todo fix this
        _q.enqueue(EventType {});
    }

protected:
    moodycamel::BlockingConcurrentQueue<
        EventType,
        moodycamel::ConcurrentQueueDefaultTraits> _q {kDefaultQueueSize};

    std::atomic_bool _isRunning = ATOMIC_VAR_INIT(false);
};


template<typename EventType>
class BlockingEventHandlerQueue : public BlockingEventQueue<EventType>
{
    std::vector<detail::Dispatcher*> handlers;
public:
    auto addHandler(detail::Dispatcher* sub) -> void
    {
        handlers.push_back(sub);
    }

    virtual auto processEvent(const EventType& evt) -> void
    {
        for(const auto& handler : handlers)
        {
            if(handler->hasHandler(evt))
            {
                handler->innerDispatch(evt);
                break;
            }
        }
    }
};

template<typename Visitor, typename Variant>
class BlockingVariantQueue : public BlockingEventQueue<Variant>
{
public:
    BlockingVariantQueue() = default;

    virtual auto processEvent(const Variant& evt) -> void
    {
        std::visit(static_cast<Visitor&>(*this), evt);
    }
};

template<typename EventType>
class Handler :
    public detail::Dispatcher,
    virtual public BlockingEventHandlerQueue<EventBasePtr>
{
public:
    virtual ~Handler() = default;

    Handler() { addHandler(this); }

    auto hasHandler(EventBasePtr evt) const -> bool override
    {
        auto e = std::dynamic_pointer_cast<EventType>(evt);
        return e != nullptr;
    }

    auto doDispatch(EventBasePtr evt) -> void override
    {
        auto e = std::dynamic_pointer_cast<EventType>(evt);
        onEvent(*e);
    }

    virtual auto onEvent(const EventType& evt) -> void = 0;
};

template<typename Event>
class VariantHandler :
    public detail::Dispatcher,
    public BlockingEventQueue<Event>
{
public:
    virtual ~VariantHandler() = default;

    virtual auto processEvent(const Event& evt) -> void override
    {
        // std::visit(&v, evt);
    }

    // Visitor v;
};

} // namespace mla::event

#endif

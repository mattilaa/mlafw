#ifndef __MLA_EVENT_H__
#define __MLA_EVENT_H__

#include <assert.h>
#include <map>
#include <typeindex>

namespace mla::event {

class EventBase
{
public:
    virtual ~EventBase() = default;
};

namespace details {

class Dispatcher
{
public:
    virtual ~Dispatcher() = default;

private:
    friend class SubBase;
    virtual auto doDispatch(const EventBase& evt) -> void = 0;
    auto innerDispatch(const EventBase& evt) -> void { doDispatch(evt); }
};

class SubBase
{
    std::map<std::type_index, Dispatcher*> handlers;
public:
    virtual ~SubBase() = default;

    template<typename E>
    auto dispatch(const E& evt) -> void
    {
        auto id = std::type_index(typeid(evt));
        if(auto it = handlers.find(id); it != handlers.end())
            it->second->innerDispatch(evt);
        else // \todo what here? now just assert
            assert(false);
    }

protected:
    auto addHandler(const std::type_index& info, Dispatcher* sub) -> void
    {
        handlers[info] = sub;
    }
};

} // namespace details

template<typename EventType>
class Sub : public details::Dispatcher, virtual public details::SubBase
{
public:
    virtual ~Sub() = default;

    Sub() { addHandler(std::type_index(typeid(EventType)), this); }

    virtual auto doDispatch(const EventBase& evt) -> void override
    {
        auto e = static_cast<const EventType&>(evt);
        onEvent(e);
    }

    virtual auto onEvent(const EventType& evt) -> void = 0;
};

} // namespace mla::event

#endif

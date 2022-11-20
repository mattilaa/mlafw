#ifndef __MLA_EVENT_DETAIL_H__
#define __MLA_EVENT_DETAIL_H__

#include <assert.h>
#include <map>
#include <typeindex>

#include <iostream>

#include "mlafw/mladefs.h"

#include "blockingconcurrentqueue.h"

namespace mla::event {

static constexpr int kDefaultQueueSize = 10000;

class EventBase
{
public:
    virtual ~EventBase() = default;
};

template<typename T>
class Visitor
{
public:
    virtual ~Visitor() = default;

    virtual auto operator()(const T&) -> void = 0;
};

using EventBasePtr = std::shared_ptr<EventBase>;

namespace detail {

class Dispatcher
{
public:
    virtual ~Dispatcher() = default;

    virtual auto hasHandler(EventBasePtr evt) const -> bool = 0;
    virtual auto doDispatch(EventBasePtr evt) -> void = 0;
    auto innerDispatch(EventBasePtr evt) -> void { doDispatch(evt); }
};

}
} // namespace mla::event

#endif

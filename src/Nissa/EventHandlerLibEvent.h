#ifndef THORSANVIL_NISSA_EVENT_HANDLER_LIBEVENT_H
#define THORSANVIL_NISSA_EVENT_HANDLER_LIBEVENT_H

#include "NissaConfig.h"
#include <event2/event.h>
#include <utility>

namespace ThorsAnvil::Nissa
{

using LibEventEventBase     = ::event_base;
using LibEventEvent         = ::event;
using LibEventTimeOut       = ::timeval;

class EventHandler;

class Event;
class EventBase
{
    friend class Event;
    LibEventEventBase*      eventBase;
    public:
        EventBase()
            : eventBase(event_base_new())
        {}
        ~EventBase()
        {
            event_base_free(eventBase);
        }

        EventBase(EventBase const&)                 = delete;
        EventBase(EventBase&&)                      = delete;
        EventBase& operator=(EventBase const&)      = delete;
        EventBase& operator=(EventBase&&)           = delete;

        void run()
        {
            event_base_loop(eventBase, EVLOOP_NO_EXIT_ON_EMPTY);
        }
};

class Event
{
    LibEventEvent*          event;

    public:
        Event(EventBase& eventBase, EventHandler& eventHandler);
        Event(EventBase& eventBase, int fd, short type, EventHandler& eventHandler);

        Event(Event&& move)
            : event(std::exchange(move.event, nullptr))
        {}
        Event& operator=(Event&& move)
        {
            event = std::exchange(move.event, nullptr);
            return *this;
        }
        ~Event()
        {
            if (event) {
                event_free(event);
            }
        }
        Event(Event const&)                         = delete;
        Event& operator=(Event const&)              = delete;

        void add()
        {
            event_add(event, nullptr);
        }
        void add(int microsecondsPause)
        {
            LibEventTimeOut timeout = {0, microsecondsPause};
            evtimer_add(event, &timeout);
        }
};


}

#endif

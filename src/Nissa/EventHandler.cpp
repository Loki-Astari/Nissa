#include "EventHandler.h"
#include "JobQueue.h"

/*
 * C Callback functions.
 * Simply decide the data into EventHandler and call the C++ functions.
 */
void eventCallback(evutil_socket_t fd, short eventType, void* data)
{
    ThorsAnvil::Nissa::EventHandler&    eventHandler = *reinterpret_cast<ThorsAnvil::Nissa::EventHandler*>(data);
    eventHandler.eventHandle(fd, static_cast<ThorsAnvil::Nissa::EventType>(eventType));
}

void controlTimerCallback(evutil_socket_t, short, void* data)
{
    ThorsAnvil::Nissa::EventHandler&    eventHandler = *reinterpret_cast<ThorsAnvil::Nissa::EventHandler*>(data);
    eventHandler.controlTimerAction();
}

using namespace ThorsAnvil::Nissa;

/*
 * EventLib wrapper. Set up C-Function callbacks
 */
Event::Event(EventBase& eventBase, int fd, short type, EventHandler& eventHandler)
    : event{event_new(eventBase.eventBase, fd, type, &eventCallback, &eventHandler)}
{}

Event::Event(EventBase& eventBase, EventHandler& eventHandler)
    : event{evtimer_new(eventBase.eventBase, controlTimerCallback, &eventHandler)}
{}

EventHandler::EventHandler(JobQueue& jobQueue)
    : jobQueue(jobQueue)
    , timer(eventBase, *this)
{
    timer.add(ControlTimerPause);
}

void EventHandler::run()
{
    eventBase.run();
}

void EventHandler::add(int fd, ThorsAnvil::ThorsSocket::SocketStream&& stream, EventAction&& action)
{
    std::unique_lock    lock(updateListMutex);
    updateList.emplace_back(fd, true, EventTask::RestoreRead, std::move(action), std::move(stream));
}

bool EventHandler::checkFileDescriptorOK(int fd, EventType type)
{
    /*
     * This function detects if the socket has been closed at the other end.
     * This will induce a read event but there will be no data on the stream.
     */
    if (type == EventType::Write) {
        return true;
    }
    char buffer;
    ssize_t result = recv(fd, &buffer, 1, MSG_PEEK);
    return !(result == 0 || (result == -1 && errno != EAGAIN && errno != EWOULDBLOCK));
}

void EventHandler::eventHandle(int fd, EventType type)
{
    auto find = tracking.find(fd);
    if (find != tracking.end())
    {
        EventInfo& info = find->second;

        /*
         * If the socket was cloes.
         * Then remove it and all its data.
         */
        if (info.stream.getSocket().isConnected() && !checkFileDescriptorOK(fd, type))
        {
            std::cerr << "Remove Socket\n";
            std::unique_lock    lock(updateListMutex);
            updateList.emplace_back(fd, false, EventTask::Remove, EventAction{}, ThorsAnvil::ThorsSocket::SocketStream{});
            return;
        }
        /*
         * Add a lamda to the JobQueue to read/write data from the stream
         * using the stored "EventAction via the CoRoutine.
         */
        jobQueue.addJob([&]()
        {
            EventTask task = info.action(info.stream);

            std::unique_lock    lock(updateListMutex);
            updateList.emplace_back(fd, false, task, EventAction{}, ThorsAnvil::ThorsSocket::SocketStream{});
        });
    }
}

/*
 * Callback function controlled by timer.
 * This is used to processes all pending changes.
 *
 * Thus all changes to state are done by the main thread.
 */
void EventHandler::controlTimerAction()
{
    std::unique_lock    lock(updateListMutex);
    for (auto& eventUpdate: updateList)
    {
        auto find = tracking.find(eventUpdate.fd);
        // If this is a create then set up all the data.
        if (eventUpdate.create)
        {
            if (find == tracking.end())
            {
                Event readEvent(eventBase, eventUpdate.fd, EV_READ, *this);
                Event writeEvent(eventBase, eventUpdate.fd, EV_WRITE, *this);
                auto const [iter, ok] = tracking.insert({eventUpdate.fd, EventInfo{std::move(readEvent), std::move(writeEvent), std::move(eventUpdate.action), std::move(eventUpdate.stream)}});
                find = iter;
            }
            else
            {
                find->second.action = std::move(eventUpdate.action);
                find->second.stream = std::move(eventUpdate.stream);
            }
        }
        if (find == tracking.end()) {
            continue;
        }

        // Now set any listeners.
        switch (eventUpdate.task)
        {
            case EventTask::RestoreRead:
            {
                find->second.readEvent.add();
                break;
            }
            case EventTask::RestoreWrite:
            {
                find->second.writeEvent.add();
                break;
            }
            case EventTask::Remove:
            {
                tracking.erase(find);
                break;
            }
        }
    }
    updateList.clear();
    // Put the timer back.
    timer.add(ControlTimerPause);
}

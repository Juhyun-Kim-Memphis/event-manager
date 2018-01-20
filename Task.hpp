#ifndef EVENT_MANAGER_TASK_HPP
#define EVENT_MANAGER_TASK_HPP

#include <unistd.h>
#include <map>
#include "Pipe.hpp"
#include "Event.hpp"

struct EventAndHandler {
    EventAndHandler(Event *event, const std::function<void()> &handle) : event(event), handle(handle) {}

    Event *event; /* TODO: check if this leaks. (should we use smart pointer?) */
    std::function<void()> handle;
};

class Task {
public:
    class UnimplementedHandle : public std::exception {
    public:
        const char* what() const noexcept {return "UnimplementedHandle!\n";}
    };

    Task() :  state(INITIAL) {}

    virtual void start() = 0;
    virtual void handle(Message *msg) {
        if(eventMap.find(msg->getType()) == eventMap.end())
            throw UnimplementedHandle();

        EventAndHandler eventAndHandler = eventMap[msg->getType()](msg);
        /* TODO: eventQueue must shore this EventAndHandler here? */
        eventAndHandler.handle();
    };
    bool hasQuit();

    /* TODO: make public method to force to quit the task */

protected:
    void quit();
    void setFactoryAndHandlerFor(Event::EventType type, const std::function<EventAndHandler(Message *)> &FactoryAndHandler) {
        eventMap[type] = FactoryAndHandler;
    }

    template<typename E, typename T>
    void setFactoryAndHandlerForEventType(T *derivedTask){
        setFactoryAndHandlerFor(E::eventType(), [derivedTask](Message *msg){
            E *event = E::makeEventNew(*msg);
            void (T::*fn)(E *) = &T::handleEvent;
            return EventAndHandler(event, std::bind(fn, derivedTask, event));
        }); /* TODO: how long the life cycle of this lambda is ? */
    }

    enum State {
        INITIAL,
        TERMINATED
    };
    State state; //TODO: make TaskState class

private:
    std::map<Event::EventType, std::function<EventAndHandler(Message *)>> eventMap;
};

#endif //EVENT_MANAGER_TASK_HPP

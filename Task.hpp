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
    class NotImplementedHandle : public std::exception {
    public:
        const char* what() const noexcept {return "NotImplementedHandle!\n";}
    };

    Task() :  taskFinished(false) {}

    virtual void start() = 0;
    virtual void handle(Message *msg) {
        if(eventMap.find(msg->getID()) == eventMap.end()){
            throw NotImplementedHandle();
        }

        EventAndHandler eventAndHandler = eventMap[msg->getID()](msg);
        /* TODO: eventQueue must shore this EventAndHandler here? */
        eventAndHandler.handle();
        delete eventAndHandler.event;
    };

    virtual void dumpDebugInfo() { /* TODO: take Stream& parameter using Template */
        std::cerr<<"task dump."<<std::endl;
    }

    bool hasQuit() const;

    /* TODO: make public method to force to quit the task */
    /* TODO: make ExpectingEventQueue, which will prevent quiting when it has some events expected. */

protected:
    void quit();
    template<typename E, typename T>
    void useDefaultHandler(T *derivedTask){
        addToDefaultEventMap(E::getMessageID(), [derivedTask](Message *msg) {
            E *event = E::newEvent(*msg);
            void (T::*fn)(E *) = &T::handleEvent;
            return EventAndHandler(event, std::bind(fn, derivedTask, event));
        }); /* TODO: how long the life cycle of this lambda is ? */
    }

private:
    bool taskFinished;

    std::map<Message::TypeID, std::function<EventAndHandler(Message *)>> eventMap;

    void addToDefaultEventMap(Message::TypeID type, const std::function<EventAndHandler(Message *)> &FactoryAndHandler) {
        eventMap[type] = FactoryAndHandler;
    }
};

/* TODO: make StaticallyPlanedTask which has planed Event list(?)
 * and call quit automatically when it execute the planed Event list.  */

#endif //EVENT_MANAGER_TASK_HPP

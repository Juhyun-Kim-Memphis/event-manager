#ifndef EVENT_MANAGER_EVENT_HPP
#define EVENT_MANAGER_EVENT_HPP

/**
 * TODO: Move all of the actual implementations to cpp.
 * TODO: Expand on derived Events
 * TODO: Expand on the usability of the EventID
 * Question: Can Events be singleton??????????? Doesn't seem likely
 * If not- need to consider the cost of creating & destroying Event objects
 */
class Event {
public:
    //char array (string) under certain lenght (8byte?) seems to be a fine replacement for a character to represent the event
    typedef char EventID;

    Event() {}

    Event(EventID id_) : id(id_) {}

    EventID getEventID() { return this->id; }

    //To be removed- used only in the preliminary testing phase
    bool isQuit() { return id == 'q'; }

    //EventType seems to be a more appropriate name
    EventID id;

    bool operator==(const Event &rhs) const {
        return id == rhs.id;
    }

    bool operator!=(const Event &rhs) const {
        return !(rhs == *this);
    }
};


class QuitEvent : public Event {
public:
    //Default constructor: simply making an event with an event id of 'q'
    QuitEvent() : Event('q') {}
};

class LockEvent : public Event {
public:
    //Default constructor: simply making an event with an event id of 'q'
    LockEvent() : Event('l') {}

    //simply returns the nature of this event:
    //is this for notifying me that the lock is released? is it for something else?
    std::string checkEvent() { return this->eventFor; };
private:
    std::string eventFor;
};

#endif //EVENT_MANAGER_EVENT_HPP

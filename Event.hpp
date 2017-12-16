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
    typedef char EventType;

    Event() {}

    Event(EventType t) : type(t) {}

    EventType getEventID() const { return this->type; }

    bool isRelease() { return type == 'r'; }

    bool operator==(const Event &rhs) const {
        return type == rhs.type;
    }

    bool operator!=(const Event &rhs) const {
        return !(rhs == *this);
    }

public: //TODO: make it private and apply intrusive serialize method.
    EventType type;
};

class LockReleaseEvent : public Event {
public:
    LockReleaseEvent() : Event('r') {}

private:
};

#endif //EVENT_MANAGER_EVENT_HPP

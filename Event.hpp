#ifndef EVENT_MANAGER_EVENT_HPP
#define EVENT_MANAGER_EVENT_HPP


class Event {
public:
    typedef char EventID;

    Event(EventID id_) : id(id_) {}
    bool isQuit(){ return id == 'q'; }

    EventID id;
};

#endif //EVENT_MANAGER_EVENT_HPP

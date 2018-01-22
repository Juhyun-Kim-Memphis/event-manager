#ifndef EVENT_MANAGER_EVENT_HPP
#define EVENT_MANAGER_EVENT_HPP

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

class Event {
public:
    typedef int EventType;
    using Priority = uint32_t;

    Event(EventType t) : type(t) {}

    virtual EventType getEventID() const { return this->type; }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & priority;
    }

private:
    /* TODO: can we remove this? redundant information.. */
    Priority priority;
    EventType type;
};

#endif //EVENT_MANAGER_EVENT_HPP

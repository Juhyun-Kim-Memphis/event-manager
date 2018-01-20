#ifndef EVENT_MANAGER_EVENT_HPP
#define EVENT_MANAGER_EVENT_HPP

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

class Event {
public:
    typedef int EventType;

    Event(EventType t) : type(t) {}

    virtual EventType getEventID() const { return this->type; }

    bool operator==(const Event &rhs) const {
        return type == rhs.type;
    }

    bool operator!=(const Event &rhs) const {
        return !(rhs == *this);
    }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & type;
    }

private:
    EventType type;
    /* TODO: can we remove this? redundant information.. */
};

#endif //EVENT_MANAGER_EVENT_HPP

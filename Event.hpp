#ifndef EVENT_MANAGER_EVENT_HPP
#define EVENT_MANAGER_EVENT_HPP

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

class Event {
public:
    typedef char EventType;

    Event() {}
    Event(EventType t) : type(t) {}

    EventType getEventID() const { return this->type; }

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
};

#endif //EVENT_MANAGER_EVENT_HPP

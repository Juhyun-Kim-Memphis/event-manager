#ifndef EVENT_MANAGER_EVENT_HPP
#define EVENT_MANAGER_EVENT_HPP

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

class Event {
public:
    using Priority = uint32_t;

    Event() : priority(0) {}

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & priority;
    }

private:
    Priority priority;
};

#endif //EVENT_MANAGER_EVENT_HPP

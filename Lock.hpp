#ifndef EVENT_MANAGER_LOCK_HPP
#define EVENT_MANAGER_LOCK_HPP

#include <queue>
#include <mutex>
#include <algorithm>
#include <sstream>
#include "Event.hpp"
#include "Pipe.hpp"

#ifdef _WIN32
#include <mingw.mutex.h>
#endif
/**
 * TODO: Move all of the actual implementations to cpp.
 */
using namespace std;

class LockOwnershipChange : public Event {
public:
    LockOwnershipChange(): Event('r') {}

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<Event>(*this);
    }

    Message makeMessage() {
        std::stringbuf buf;
        std::ostream os(&buf);
        boost::archive::binary_oarchive oa(os, boost::archive::no_header);
        oa << *this;
        return Message(getEventID(), buf.str().length(), buf.str().c_str());
    }

    static LockOwnershipChange *makeFromMsg(Message &msg) {
        if(msg.getType() == 'r')
            return new LockOwnershipChange;
        else
            throw std::string("unknown EventType: ").append(std::to_string(msg.getType()));
    }
};

class Lock {
public:
    using User = const PipeWriter*;

    Lock(int id_) : id(id_), owner(nullptr), lockVal(UNLOCKED) {}
    Lock() : Lock(0) {}

    virtual bool acquire(User requester);
    virtual void release();

    User getOwner() { return owner; }

    bool isInWaiters(User u) {
        return waiters.end() != find(waiters.begin(), waiters.end(), u);
    }

    bool hasLocked() {
        mtx.lock();
        bool hasLocked = (lockVal == LOCKED);
        mtx.unlock();
        return hasLocked;
    }

protected:
    void giveLockOwnership(User waiter) {
        LockOwnershipChange event;

        lockVal = LOCKED; /* acquire lock on behalf of the waiter. */
        owner = waiter;
        waiter->writeOneMessage(event.makeMessage()); /* waiter got msg. */
    }

    enum LockValue {
        LOCKED = 1,
        UNLOCKED = 0
    };
    LockValue lockVal;
    int id;
    User owner;

    std::deque<User> waiters;

    std::mutex mtx; //for testing
};

#endif //EVENT_MANAGER_LOCK_HPP

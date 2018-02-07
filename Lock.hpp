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
 * TODO: allocate id, waiter list and owner info to shared memory.
 */
using namespace std;

class LockOwnershipChange : public Event {
public:
    static constexpr Message::TypeID getMessageID() { return msgID; }

    Message makeMessage() {
        return Message::makeDummyMessage(getMessageID());
    }

    static LockOwnershipChange *newEvent(Message &msg) {
        if(msg.getID() == getMessageID())
            return new LockOwnershipChange;
        else
            throw std::string("unknown EventType: ").append(std::to_string(msg.getID()));
    }

private:
    static constexpr Message::TypeID msgID = 4;
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
        std::lock_guard<std::mutex> guard(mtx);
        return (lockVal == LOCKED);
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

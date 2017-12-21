#ifndef EVENT_MANAGER_LOCK_HPP
#define EVENT_MANAGER_LOCK_HPP

#include <queue>
#include <mutex>
#include <algorithm>
#include "Event.hpp"

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
};

class LockUser {
public:
    LockUser(int writePipefd) : writePipefd(writePipefd) {}

    int writePipefd;
};

class Lock {
public:
    Lock(int id_) : id(id_), owner(-1), lockVal(UNLOCKED) {}
    Lock() : Lock(0) {}

    virtual bool acquire(int writePipeFdOfRequester);
    virtual void release();

    int getOwner() { return owner; }

    bool isInWaiters(int myWriteFd) {
        return waiters.end() != find(waiters.begin(), waiters.end(), myWriteFd);
    }

    bool hasLocked() {
        mtx.lock();
        bool hasLocked = (lockVal == LOCKED);
        mtx.unlock();
        return hasLocked;
    }

protected:
    void giveLockOwnership(LockUser waiter) {

        lockVal = LOCKED; /* acquire lock on behalf of the waiter. */

        LockOwnershipChange event;
        char released = event.getEventID();
        write(waiter.writePipefd, &released, 1);
    }

    enum LockValue {
        LOCKED = 1,
        UNLOCKED = 0
    };
    LockValue lockVal;
    int id;
    int owner;
    std::deque<int> waiters;

    std::mutex mtx; //for testing
};

#endif //EVENT_MANAGER_LOCK_HPP

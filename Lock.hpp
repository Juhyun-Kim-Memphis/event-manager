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



class LockUser {
public:
    LockUser(int writePipefd) : writePipefd(writePipefd) {}

    void sendEvent(LockUser to, Event input){
        char released = input.getEventID();
        write(to.writePipefd, &released, 1);
    }
private:
    int writePipefd;
};

class Lock {
public:
    Lock(int id_) : id(id_), owner(-1), lockVal(UNLOCKED) {}

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
//    typedef int LockUser;

    void giveLockOwnership(LockUser waiter);

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

class LockUsingAOP : public Lock {
public:
    explicit LockUsingAOP(int id) : lockVal(0), Lock(id) {}

    bool acquire(int writePipeFdOfRequester) override;
    void release() override;

private:
    inline long aop_cas(volatile long *vp, long nv, long ov);

    volatile long lockVal;

};

#endif //EVENT_MANAGER_LOCK_HPP

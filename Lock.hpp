#ifndef EVENT_MANAGER_LOCK_HPP
#define EVENT_MANAGER_LOCK_HPP

#include <queue>
#include <mutex>
#include <algorithm>
#ifdef _WIN32
#include <mingw.mutex.h>
#endif

/**
 * TODO: Move all of the actual implementations to cpp.
 */
class Lock {
public:
    Lock () : lockVal(UNLOCKED) {}
    Lock (std::string name) : lockVal(UNLOCKED), lockName(name) {}
    bool acquire(int writePipeFdOfRequester) {
        if(lockVal == UNLOCKED) {
            lockVal = LOCKED;
            return true;
        }
        else{
            waiters.push_back(writePipeFdOfRequester);
            return false;
        }
    }

    void release() {
        if(lockVal == UNLOCKED)
            throw std::string("releasing unlocked Lock!");

        if(waiters.empty()){
            lockVal = UNLOCKED;
        } else {
            //TODO: write lock's id to pipe.
            int waiterFd = waiters.front();
            waiters.pop_front();
            lockVal = LOCKED; /* acquire lock on behalf of the waiter. */

            char released = 'r';
            write(waiterFd, &released, 1);
        }
    }

    bool isInWaiters(int myWriteFd) {
        return waiters.end() != std::find(waiters.begin(), waiters.end(), myWriteFd);
    }

    std::string getLockName(){
        return lockName;
    }

private:
    typedef int FdOfWaiter;

    enum LockValue {
        LOCKED,
        UNLOCKED
    };

    LockValue lockVal;
    std::string lockName;
    std::deque<FdOfWaiter> waiters;
};

#endif //EVENT_MANAGER_LOCK_HPP

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
    //Default constructor setting locval as unlocked
    Lock() : lockVal(UNLOCKED) {}


    Lock(std::string name) : lockVal(UNLOCKED), lockName(name) {}

    bool acquire(int writePipeFdOfRequester) {
        if (aop_cas(&lockVal, 1, 0) == 0) {
            owner = writePipeFdOfRequester;
            return true;
        } else {
            waiters.push_back(writePipeFdOfRequester);
            return false;
        }
    }

    void release() {
        if (lockVal == UNLOCKED) {
            std::cout << "releasing unlocked lock. owner:"<< owner <<"\n";
        }

        if (waiters.empty()) {
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

    std::string getLockName() {
        return lockName;
    }

private:
    static inline long
    aop_cas(volatile long *vp, long nv, long ov) {
        asm __volatile__(
        "lock ; " "cmpxchgq %1,(%3)"
        : "=a" (nv)
        : "r" (nv), "a" (ov), "r" (vp)
        : "cc", "memory"
        );

        return nv;
    }

    typedef int FdOfWaiter;

    enum LockValue {
        LOCKED = 1,
        UNLOCKED = 0
    };

    volatile long lockVal;

    std::string lockName;
    std::deque<FdOfWaiter> waiters;
    int owner;
};

#endif //EVENT_MANAGER_LOCK_HPP

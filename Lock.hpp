#ifndef EVENT_MANAGER_LOCK_HPP
#define EVENT_MANAGER_LOCK_HPP

#include <queue>
#include <mutex>
#ifdef _WIN32
#include <mingw.mutex.h>
#endif

class Lock {
public:
    Lock () : lockVal(UNLOCKED) {}
    bool acquire(int writePipeFdOfRequester) {
        if(lockVal == UNLOCKED) {
            lockVal = LOCKED;
            return true;
        }
        else{
            waiters.push(writePipeFdOfRequester);
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
            char released = 'r';
            int waiterFd = waiters.front();
            waiters.pop();
            lockVal = LOCKED; /* acquire lock on behalf of the waiter. */
            write(waiterFd, &released, 1);
        }
    }

private:
    enum LockValue {
        LOCKED,
        UNLOCKED
    };

    LockValue lockVal;
    std::queue<int> waiters;
};

#endif //EVENT_MANAGER_LOCK_HPP

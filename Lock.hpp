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
using namespace std;
class Lock {
public:
    Lock(int id_) : id(id_), owner(-1), lockVal(UNLOCKED) {}

    virtual bool acquire(int writePipeFdOfRequester) {
        bool acquired = false;
        mtx.lock();

        if (lockVal == UNLOCKED) {
            lockVal = LOCKED;
            owner = writePipeFdOfRequester;
            acquired = true;
        }
        else{
            waiters.push_back(writePipeFdOfRequester);
            acquired = false;
        }

        mtx.unlock();
        return acquired;
    }

    virtual void release() {
        mtx.lock();
        if (lockVal == UNLOCKED){
            mtx.unlock();
            throw exception();
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
        mtx.unlock();
    }

    bool isInWaiters(int myWriteFd) {
        return waiters.end() != find(waiters.begin(), waiters.end(), myWriteFd);
    }

    int getID() {
        return id;
    }

    bool testHasLocked() {
        mtx.lock();
        bool hasLocked = (lockVal == LOCKED);
        mtx.unlock();
        return hasLocked;
    }

protected:
    enum LockValue {
        LOCKED = 1,
        UNLOCKED = 0
    };
    LockValue lockVal;
    int id;
    typedef int PipeFd;
    PipeFd owner;
    std::deque<PipeFd> waiters;

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

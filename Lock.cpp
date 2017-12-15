#include <unistd.h>
#include <iostream>
#include "Lock.hpp"

bool Lock::acquire(int writePipeFdOfRequester) {
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

void Lock::release() {
    mtx.lock();
    if (lockVal == UNLOCKED){
        mtx.unlock();
        throw exception();
    }

    if (waiters.empty()) {
        lockVal = UNLOCKED;
    } else {
        //TODO: write lock's id to pipe.
        int waiterfd = waiters.front();
        waiters.pop_front();
        LockUser waiter(waiterfd);

        giveLockOwnership(waiter);
    }
    mtx.unlock();
}

void Lock::giveLockOwnership(LockUser waiter) {
    LockUser _owner(owner);

    lockVal = LOCKED; /* acquire lock on behalf of the waiter. */

    _owner.sendEvent(waiter, Event('r'));
}

bool LockUsingAOP::acquire(int writePipeFdOfRequester) {
    if (aop_cas(&lockVal, 1, 0) == 0) {
        owner = writePipeFdOfRequester;
        return true;
    } else {
        waiters.push_back(writePipeFdOfRequester);
        return false;
    }
}

void LockUsingAOP::release() {
    /* TODO: use aop */
    Lock::release();
}

long LockUsingAOP::aop_cas(volatile long *vp, long nv, long ov) {
#ifndef _WIN32
    asm __volatile__(
    "lock ; " "cmpxchgq %1,(%3)"
    : "=a" (nv)
    : "r" (nv), "a" (ov), "r" (vp)
    : "cc", "memory"
    );
#endif
    return nv;
}

#include <unistd.h>
#include <iostream>
#include "Lock.hpp"

bool Lock::acquire(User requester) {
    bool acquired = false;
    mtx.lock();

    if (lockVal == UNLOCKED) {
        lockVal = LOCKED;
        owner = requester;
        acquired = true;
    }
    else{
        waiters.push_back(requester);
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
        owner = nullptr;
    } else {
        //TODO: write lock's id to pipe.
        User waiter = waiters.front();
        waiters.pop_front();

        giveLockOwnership(waiter);
    }
    mtx.unlock();
}

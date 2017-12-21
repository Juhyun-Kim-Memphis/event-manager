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

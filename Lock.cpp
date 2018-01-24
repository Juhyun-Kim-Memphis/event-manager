#include <unistd.h>
#include <iostream>
#include "Lock.hpp"

bool Lock::acquire(User requester) {
    bool acquired = false;
    std::lock_guard<std::mutex> guard(mtx);

    if (lockVal == UNLOCKED) {
        lockVal = LOCKED;
        owner = requester;
        acquired = true;
    }
    else{
        waiters.push_back(requester);
        acquired = false;
    }

    return acquired;
}

void Lock::release() {
    std::lock_guard<std::mutex> guard(mtx);
    if (lockVal == UNLOCKED)
        throw exception();

    if (waiters.empty()) {
        lockVal = UNLOCKED;
        owner = nullptr;
    } else {
        //TODO: write lock's id to pipe.
        User waiter = waiters.front();
        waiters.pop_front();

        giveLockOwnership(waiter);
    }
}

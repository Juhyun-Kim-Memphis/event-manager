#ifndef EVENT_MANAGER_LOCK_HPP
#define EVENT_MANAGER_LOCK_HPP

#include <queue>
#include <mutex>
#ifdef _WIN32
#include <mingw.mutex.h>
#endif

class Lock {
public:
    bool acquire(int writePipeFdOfRequester) {
        if(mtx.try_lock())
            return true;
        else{
            waiters.push(writePipeFdOfRequester);
            return false;
        }
    }

    void release() {
        char released = 'r';
        mtx.unlock(); //TODO: acquire lock for waiter.
        int waiterFd = waiters.front();
        waiters.pop();
        write(waiterFd, &released, 1);
    }

private:
    std::mutex mtx;
    std::queue<int> waiters;
};

#endif //EVENT_MANAGER_LOCK_HPP

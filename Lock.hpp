#ifndef EVENT_MANAGER_LOCK_HPP
#define EVENT_MANAGER_LOCK_HPP

#include <mutex>
#ifdef _WIN32
#include <mingw.mutex.h>
#endif

class Lock {
public:
    bool acquire() {
        return mtx.try_lock();
    }

    void release() {
        mtx.unlock();
    }

private:
    std::mutex mtx;
};

#endif //EVENT_MANAGER_LOCK_HPP

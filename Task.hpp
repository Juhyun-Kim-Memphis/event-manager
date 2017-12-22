#ifndef EVENT_MANAGER_TASK_HPP
#define EVENT_MANAGER_TASK_HPP

#include <unistd.h>
#include "Pipe.hpp"
#include "Event.hpp"
#include "Lock.hpp"

using namespace std;

class Task {
public:
    Task(PipeWriter &pw) : lockUser(pw), state(INITIAL) {}

    virtual void start() = 0;
    virtual void handle(Event event) {};
    void quit(); // TODO: make it protected
    bool hasQuit();

protected:
    enum State {
        INITIAL,
        TERMINATED
    };
    PipeWriter &lockUser;
    State state; //TODO: make TaskState class
};

class LockAcquireTask : public Task {
public:
    LockAcquireTask(PipeWriter &pw, vector<Lock*> requiredLocks)
            : Task(pw), requiredLocks(std::move(requiredLocks)) {}

    void start() override {
        Lock::User myself = &lockUser;

        for(Lock *lock : requiredLocks){
            if(lock->acquire(myself))
                acquiredLocks.push_back(lock);
        }
        quit();
    }

    vector<Lock*> getAwaitedLocks() {
        Lock::User myself = &lockUser;
        vector<Lock*> awaitedLocks;

        for(Lock *lock : requiredLocks){
            if(lock->isInWaiters(myself))
                awaitedLocks.push_back(lock);
        }

        return awaitedLocks;
    }

    vector<Lock*> getAcquiredLocks() {
        return acquiredLocks;
    }

private:
    const vector<Lock*> requiredLocks;
    vector<Lock*> acquiredLocks;
};





#endif //EVENT_MANAGER_TASK_HPP

#ifndef EVENT_MANAGER_TASK_HPP
#define EVENT_MANAGER_TASK_HPP

#include <unistd.h>
#include "Module.hpp"
#include "Pipe.hpp"
#include "Event.hpp"

using namespace std;

class Task {
public:
    Task(int wfd) : writePipeFd(wfd) {}

    virtual void start() = 0;
    virtual void handle(Event event) {};
    int getWritePipeFd(){ return writePipeFd; }
    void quit() const;

protected:
    int writePipeFd;
};

class ModifyTask : public Task {
public:
    ModifyTask(int wfd, Module &m)
            : Task(wfd), module(m), numOfEventHandlerCalls(0) {}

    void start() override;
    void handle(Event event) override;
    int getNumOfEventHandlerCalls() { return numOfEventHandlerCalls; }

private:
    void changeModule();

    int numOfEventHandlerCalls;
    Module &module;
};

class LockAcquireTask : public Task {
public:
    LockAcquireTask(int wfd, Lock *a, Lock *b)
            : Task(wfd), lockA(a), lockB(b) {}

    void start() override {
        char quit = 'q';
        if (lockA->acquire(writePipeFd))
            acquiredLocks.push_back(lockA);

        if (lockB->acquire(writePipeFd))
            acquiredLocks.push_back(lockB);

        write(writePipeFd, &quit, 1);
    }

    vector<Lock *> getAwaitedLocks() {
        int myself = writePipeFd;
        vector<Lock *> awaitedLocks;

        if (lockA->isInWaiters(myself))
            awaitedLocks.push_back(lockA);
        if (lockB->isInWaiters(myself))
            awaitedLocks.push_back(lockB);
        return awaitedLocks;
    }

    vector<Lock *> getAcquiredLocks() {
        return acquiredLocks;
    }

private:
    Lock *lockA;
    Lock *lockB;
    vector<Lock *> acquiredLocks;
};


#endif //EVENT_MANAGER_TASK_HPP

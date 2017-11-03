#ifndef EVENT_MANAGER_TASK_HPP
#define EVENT_MANAGER_TASK_HPP

#include <unistd.h>
//#include <string>
#include "Module.hpp"
#include "Pipe.hpp"
#include "Event.hpp"

using namespace std;

class Task {
public:
    Task(int wfd) : writePipeFd(wfd) {}

    virtual void start() = 0;

    std::function<void(void)> eventHandle;

//    void eventHandle(Event *ev){
//
//    }
    virtual void handle(Event event) {};

protected:
    void quit() const;

    int writePipeFd;
};

class ModifyTask : public Task {
public:
    ModifyTask(int wfd, int nv, Module &m);

    void start() override;

    void modifySharedVarModule();

    void handle(Event event) override;

private:
    int newValueForModule;
    Module &module;

};

class LockReleasingTask : public Task {
public:
    LockReleasingTask(int wfd, Module &m) : Task(wfd), module(m) {};

    void start() override {
        // TODO: make quit Event factory call
        module.lock.acquire(writePipeFd);
        sleep(1);
        module.lock.release();
        quit();
    }

private:
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

//    void handle(Event event) override;

private:
    Lock *lockA;
    Lock *lockB;
    vector<Lock *> acquiredLocks;
};


#endif //EVENT_MANAGER_TASK_HPP

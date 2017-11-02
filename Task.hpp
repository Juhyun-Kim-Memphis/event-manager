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
    virtual void start() = 0;
    std::function<void(void)> eventHandle;
//    void eventHandle(Event *ev){
//
//    }
    virtual void handle(Event event) {};
};

class ModifyTask : public Task {
public:
    ModifyTask(int wfd, int nv, Module &m);

    void start() override;
    void modifySharedVarModule();
    void handle(Event event) override;

private:
    int writePipeFd;
    int newValueForModule;
    Module &module;
};

class LockReleasingTask : public Task {
public:
    LockReleasingTask(int wfd, Module &m) : writePipeFd(wfd), module(m) {} ;

    void start() override {
        // TODO: make quit Event factory call
        char quit = 'q';
        acquireLock();
        sleep(2);
        module.lock.release();
        write(writePipeFd, &quit, 1);
    }

private:
    void acquireLock() {
        module.lock.acquire(writePipeFd);
    }

    int writePipeFd;
    Module &module;
};

class LockAcquireTask : public Task {
public:
    LockAcquireTask(int wfd, ModuleHavingTwoSharedVar &m)
            : writePipeFd(wfd), module(m) {
    }

    void start() override {
        char quit = 'q';
        if(module.lockA.acquire(writePipeFd))
            myLocks.push_back(&module.lockA);

        if(module.lockB.acquire(writePipeFd))
            myLocks.push_back(&module.lockB);

        write(writePipeFd, &quit, 1);
    }

    vector<Lock *> getLocks () {
        return myLocks;
    }

private:
    int writePipeFd;
    ModuleHavingTwoSharedVar &module;
    vector<Lock *> myLocks;
};

class MultiLockWaitTask : public Task {
public:
    MultiLockWaitTask(int wfd, ModuleHavingTwoSharedVar &m)
            : writePipeFd(wfd), module(m), waitingLocks()  {}

    void start() override {
        char quit = 'q';
        module.lockA.acquire(writePipeFd);
        module.lockB.acquire(writePipeFd);
        write(writePipeFd, &quit, 1);
    }

    vector<Lock *> testWaitingLock(){
        if(module.lockA.isInWaiters(writePipeFd))
            waitingLocks.push_back(&module.lockA);
        if(module.lockB.isInWaiters(writePipeFd))
            waitingLocks.push_back(&module.lockB);
        return waitingLocks;
    }

private:
    int writePipeFd;
    ModuleHavingTwoSharedVar &module;
    vector<Lock *> waitingLocks;
};



#endif //EVENT_MANAGER_TASK_HPP

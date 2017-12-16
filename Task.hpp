#ifndef EVENT_MANAGER_TASK_HPP
#define EVENT_MANAGER_TASK_HPP

#include <unistd.h>
#include "Module.hpp"
#include "Pipe.hpp"
#include "Event.hpp"

using namespace std;

class TaskQuitEvent;

class Task {
public:
    Task(int wfd) : writePipeFd(wfd) {}

    virtual void start() = 0;
    virtual void handle(Event event) {};
    int getWritePipeFd(){ return writePipeFd; }
    void quit(); // TODO: make it protected

    bool hasQuit();

protected:
    int writePipeFd; //TODO: change to LockUser
    int state; //TODO: make TaskState class
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
    LockAcquireTask(int wfd, vector<Lock*> requiredLocks)
            : Task(wfd), requiredLocks(std::move(requiredLocks)) {}

    void start() override {
        for(Lock *lock : requiredLocks){
            if(lock->acquire(writePipeFd))
                acquiredLocks.push_back(lock);
        }
        quit();
    }

    vector<Lock*> getAwaitedLocks() {
        int myself = writePipeFd;
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

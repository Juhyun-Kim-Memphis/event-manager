#ifndef EVENT_MANAGER_TASK_HPP
#define EVENT_MANAGER_TASK_HPP

#include <unistd.h>
#include "Module.hpp"
#include "Pipe.hpp"
#include "Event.hpp"

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
    explicit LockReleasingTask(int wfd, Module &m) : writePipeFd(wfd), module(m) {} ;

    void start() override {
        // TODO: make quit Event factory call
        char quit = 'q';
        acquireLock();
        sleep(1);
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



#endif //EVENT_MANAGER_TASK_HPP

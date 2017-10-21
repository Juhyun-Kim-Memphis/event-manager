#ifndef EVENT_MANAGER_TASK_HPP
#define EVENT_MANAGER_TASK_HPP

#include "Module.hpp"

class ModifyTask {
public:
    ModifyTask(int nv, Module &m);

    void start();

private:
    int newValueForModule;
    Module &module;
};

class LockReleasingTask {
public:
    LockReleasingTask(Module &m) : module(m) {} ;

    void acquireLock() {
        module.lock.acquire();
    }

private:
    Module &module;
};

void workerMain(ModifyTask &task);


#endif //EVENT_MANAGER_TASK_HPP

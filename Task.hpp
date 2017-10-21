#ifndef EVENT_MANAGER_TASK_HPP
#define EVENT_MANAGER_TASK_HPP

#include <unistd.h>
#include "Module.hpp"
#include "Pipe.hpp"

class Task {
public:
    virtual void start() = 0;
    std::function<void(Task&)> eventHandle;
};

class ModifyTask : public Task {
public:
    ModifyTask(int nv, Module &m);

    void start() override;
    void modify();

private:
    int newValueForModule;
    Module &module;

    void modifySharedVarModule();
};

class LockReleasingTask : public Task {
public:
    explicit LockReleasingTask(Module &m) : module(m) {} ;

    void start() override {
        char quit = 'q';
        acquireLock();
        sleep(2);
        module.lock.release();
        write(globalPipes->writePipe2, &quit, 1);
    }

private:
    void acquireLock() {
        module.lock.acquire(globalPipes->writePipe2);
    }

    Module &module;
};

void workerMain(int readFd, Task &task);


#endif //EVENT_MANAGER_TASK_HPP

#include <iostream>
#include "Task.hpp"

ModifyTask::ModifyTask(int nv, Module &m) : module(m) {
    newValueForModule = nv;
}

void ModifyTask::start() {
    //acquire lock of module
    if(module.lock.acquire(globalPipes->writePipe1))
        modifySharedVarModule();
    else {
        std::cout<<"Fail to acquire Lock\n";
        eventHandle = &Task::start; //TODO: put  ModifyTask::modify as event handler
    }

}

void ModifyTask::modifySharedVarModule() {
    char quit = 'q';

    module.sharedVar = newValueForModule;
    module.lock.release();

    write(globalPipes->writePipe1, &quit, 1);
}

void workerMain(int readFd, Task &task) {
    char pipeBuf[64];
    task.start();

    while (read(readFd, &pipeBuf, 1) > 0){
        if(pipeBuf[0] == 'q')
            break;
        else if(pipeBuf[0] = 'r')
            task.eventHandle(task);
    }
}

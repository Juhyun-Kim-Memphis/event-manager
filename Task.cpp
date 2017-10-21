#include <iostream>
#include "Task.hpp"

ModifyTask::ModifyTask(int wfd, int nv, Module &m) : module(m) {
    newValueForModule = nv;
    writePipeFd = wfd;
}

void ModifyTask::start() {
    //acquire lock of module
    if(module.lock.acquire(writePipeFd))
        modifySharedVarModule();
    else {
        std::cout<<"Fail to acquire Lock\n";
        eventHandle = std::bind(&ModifyTask::modifySharedVarModule, this);
    }
}

void ModifyTask::modifySharedVarModule() {
    char quit = 'q';
    module.sharedVar = newValueForModule;
    module.lock.release();
    write(writePipeFd, &quit, 1);
}

void ModifyTask::handle(Event event) {
    if(event.id == 'r')
        modifySharedVarModule();
}

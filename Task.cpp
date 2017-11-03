#include <iostream>
#include "Task.hpp"

void Task::quit() const {
    char quit = 'q';
    write(writePipeFd, &quit, 1);
}

ModifyTask::ModifyTask(int wfd, int nv, Module &m) : Task(wfd), module(m) {
    newValueForModule = nv;
}

void ModifyTask::start() {
    //acquire lock of module
    if (module.lock.acquire(writePipeFd)) {
        modifySharedVarModule();
        quit();
    } else {
        std::cout << "Fail to acquire Lock\n";
        eventHandle = std::bind(&ModifyTask::modifySharedVarModule, this);
    }
}

void ModifyTask::modifySharedVarModule() {
    module.sharedVar = newValueForModule;
    module.lock.release();
}

void ModifyTask::handle(Event event) {
    if (event.id == 'r') {
        modifySharedVarModule();
        quit();
    }
}



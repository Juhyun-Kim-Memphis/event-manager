#include <iostream>
#include "Task.hpp"
#define TERMINATED 0

void Task::quit() {
    state = TERMINATED;
}

bool Task::hasQuit() {
    return state == TERMINATED;
}

void ModifyTask::start() {
    if(module.lock.acquire(writePipeFd)){
        changeModule();
        quit();
    }
}

void ModifyTask::changeModule() {
    module.changeIt();
    module.lock.release();
}

void ModifyTask::handle(Event event) {
    numOfEventHandlerCalls++;
    if (event.isRelease()) {
        changeModule();
        quit();
    }
}


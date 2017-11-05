#include <iostream>
#include "Task.hpp"

void Task::quit() const {
    char quit = 'q';
    write(writePipeFd, &quit, 1);
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
    if (event.id == 'r') {
        changeModule();
        quit();
    }
}


#include "Task.hpp"

ModifyTask::ModifyTask(int nv, Module &m) : module(m) {
    newValueForModule = nv;
}

void ModifyTask::start() {

    //acquire lock of module
    module.lock.acquire();
    module.sharedVar = newValueForModule;
    module.lock.release();

    return;
}

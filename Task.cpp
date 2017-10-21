#include "Task.hpp"

ModifyTask::ModifyTask(int nv, Module &m) : module(m) {
    newValueForModule = nv;
}

void ModifyTask::start() {

    //acquire lock of module
    if(module.lock.acquire())
        module.sharedVar = newValueForModule;
    else
        return;
    module.lock.release();

    return;
}

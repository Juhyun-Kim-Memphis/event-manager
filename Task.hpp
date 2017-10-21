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



#endif //EVENT_MANAGER_TASK_HPP

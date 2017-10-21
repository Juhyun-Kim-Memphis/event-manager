#ifndef EVENT_MANAGER_MODULE_HPP
#define EVENT_MANAGER_MODULE_HPP

#include "Lock.hpp"

class Module {
public:
    Module(int initialSharedVar) {
        sharedVar = initialSharedVar;
    }

    int getSharedVar() { return sharedVar; }
    void setSharedVar(int newVal) {
        sharedVar = newVal;
    }

    Lock lock;
    int sharedVar;
};

#endif //EVENT_MANAGER_MODULE_HPP

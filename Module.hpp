#ifndef EVENT_MANAGER_MODULE_HPP
#define EVENT_MANAGER_MODULE_HPP

#include "Lock.hpp"

class Module {
public:
    Module(int initialSharedVar) : lock(1) {
        sharedVar = initialSharedVar;
    }

    int getSharedVar() { return sharedVar; }

    Lock lock;
    int sharedVar;
};

#endif //EVENT_MANAGER_MODULE_HPP

#ifndef EVENT_MANAGER_MODULE_HPP
#define EVENT_MANAGER_MODULE_HPP

#include "Lock.hpp"

class Module {
public:
    Module(int initialSharedVar) {
        sharedVar = initialSharedVar;
    }

    int getSharedVar() { return sharedVar; }

    Lock lock;
    int sharedVar;
};

class ModuleHavingTwoSharedVar {
public:
    ModuleHavingTwoSharedVar(int varA, int varB)
            : lockA("lock A"), lockB("lock B") {
        sharedVarA = varA;
        sharedVarB = varB;
    }

    Lock lockA;
    int sharedVarA;

    Lock lockB;
    int sharedVarB;
};

#endif //EVENT_MANAGER_MODULE_HPP

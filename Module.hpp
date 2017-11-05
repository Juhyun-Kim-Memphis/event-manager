#ifndef EVENT_MANAGER_MODULE_HPP
#define EVENT_MANAGER_MODULE_HPP

#include "Lock.hpp"

class Module {
public:
    Module() : lock(1) {
        changed = false;
    }

    int hasChanged() { return changed; }
    void changeIt() { changed = true; }

    Lock lock;
    bool changed;
};

#endif //EVENT_MANAGER_MODULE_HPP

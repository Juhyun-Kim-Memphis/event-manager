#include <iostream>
#include "Task.hpp"
#define TERMINATED 0

void Task::quit() {
    state = TERMINATED;
}

bool Task::hasQuit() {
    return state == TERMINATED;
}


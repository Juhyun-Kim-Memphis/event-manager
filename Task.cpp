#include <iostream>
#include "Task.hpp"

void Task::quit() {
    state = TERMINATED;
}

bool Task::hasQuit() {
    return state == TERMINATED;
}


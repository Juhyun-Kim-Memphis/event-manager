#include "Task.hpp"

void Task::quit() {
    taskFinished = true;
}

bool Task::hasQuit() const {
    return taskFinished;
}


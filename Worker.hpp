#ifndef EVENT_MANAGER_WORKER_HPP
#define EVENT_MANAGER_WORKER_HPP

#include "Task.hpp"
#include "Event.hpp"

class Worker {
public:
    //TODO: remove task argument and get task via pipe. (use state pattern?)
    //TODO: Dynamic task assignment for workers - at the moment ,
    Worker(int rfd, Task *t) : readPipefd(rfd), currentTask(t) {}

    void mainLoop() {
        currentTask->start();

        while( !currentTask->hasQuit() ) {
            Event event = waitAndGetEvent();
            currentTask->handle(event);
        }
    }

private:
    Event waitAndGetEvent() {
        char buf[64];
        //TODO: read until pipe buffer gets empty.
        while (read(readPipefd, &buf, 1) > 0) {
            return Event(buf[0]);
        }

        //Need to check if this throw works properly
        throw std::string("[ERROR]Pipe Read Performed Incorrectly!");
    }

    int readPipefd;
    Task *currentTask;
};

#endif //EVENT_MANAGER_WORKER_HPP

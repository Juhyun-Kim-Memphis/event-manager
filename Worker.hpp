#ifndef EVENT_MANAGER_WORKER_HPP
#define EVENT_MANAGER_WORKER_HPP

#include "Task.hpp"
#include "Event.hpp"

class Worker {
public:
    //TODO: remove task argument and get task via pipe. (use state pattern?)
    Worker(int rfd, Task *t) : readPipefd(rfd), currentTask(t) {}

    void mainLoop(){

        currentTask->start();
        Event event = waitAndGetEvent();
        while( !event.isQuit() ){
            currentTask->handle(event);
            event = waitAndGetEvent();
        }
    }

private:
    Event waitAndGetEvent() {
        char buf[64];
        //TODO: read until pipe buffer gets empty.
        while (read(readPipefd, &buf, 1) > 0) {
            return Event(buf[0]);
        }
    }

    int readPipefd;
    Task *currentTask;
};

#endif //EVENT_MANAGER_WORKER_HPP

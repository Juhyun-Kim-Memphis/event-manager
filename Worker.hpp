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

        //Worker starts working on the first task given at the creation of the worker object
        currentTask->start();

        //Once done with the first task, simply wait for a next event
        Event event = waitAndGetEvent();

        //Once an event is received, and as long as the event received is not a QuitEvent
        //while(event.getEventID() != 'q') <-- should be replaced with this
        while (!event.isQuit()) {
            //put the event received to current task's event handler
            currentTask->handle(event);
            //wait for a next event to happen
            event = waitAndGetEvent();
        }
        //quit() should come here?
        //What does the quit() mean? : 1) The task is done and over / 2) This worker object is done with working with this task
        //If 1), quit() should be inside of the task, and worker needs to handle when the event is quit (e.g., wait for a next task)
        //If 2), quit() should come here and Task termination functionality should be implemented
        //1) seems to be what we need to implement- instead of calling join() to force quit workers
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

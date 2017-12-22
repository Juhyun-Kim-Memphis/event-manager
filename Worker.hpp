#ifndef EVENT_MANAGER_WORKER_HPP
#define EVENT_MANAGER_WORKER_HPP

#include "Task.hpp"
#include "Event.hpp"

class Worker {
public:
    //TODO: remove task argument and get task via pipe. (use state pattern?)
    //TODO: Dynamic task assignment for workers - at the moment ,
    Worker(PipeReader &pr, Task *t) : pipeReader(pr), currentTask(t) {}

    void mainLoop() {
        currentTask->start();

        while( !currentTask->hasQuit() ) {
            Event event = waitAndGetEvent();
            currentTask->handle(event);
            throw std::string("CANNOT REACH HERE.");
        }
    }

private:
    Event waitAndGetEvent() {
        //Need to check if this throw works properly
        throw std::string("[ERROR]Pipe Read Performed Incorrectly!");
    }

    PipeReader &pipeReader;
    Task *currentTask;
};

#endif //EVENT_MANAGER_WORKER_HPP

#ifndef EVENT_MANAGER_WORKER_HPP
#define EVENT_MANAGER_WORKER_HPP

#include "Task.hpp"
#include "Event.hpp"

class Worker {
public:
    //TODO: remove task argument and get task via pipe. (use state pattern?)
    /* TODO: workerMain shouldn't get any arguments except its read pipe. */
    //TODO: Dynamic task assignment for workers - at the moment ,
    Worker(PipeReader &pr, Task *t) : pipeReader(pr), currentTask(t) {}

    void mainLoop() {
        currentTask->start();

        while( !currentTask->hasQuit() ) {
            Message *msg = waitAndGetMessage();
            currentTask->handle(msg);
        }
    }

private:
    /* TODO: make return type to unique_ptr */
    Message *waitAndGetMessage() {
        try{
            return pipeReader.readOneMessage();
        } catch (const std::exception& ex) {
            std::cout<<"Exception at waitAndGetMessage: "<<ex.what()<<std::endl;
            exit(1);
        }
    }

    PipeReader &pipeReader;
    Task *currentTask;
};

#endif //EVENT_MANAGER_WORKER_HPP

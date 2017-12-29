#ifndef EVENT_MANAGER_WORKER_HPP
#define EVENT_MANAGER_WORKER_HPP

#include <thread>
#include "Task.hpp"
#include "Event.hpp"






class Worker {
public:
    class StopTask : public std::exception {
    public:
        const char* what() const noexcept {return "Stop The current task.\n";}
    };

    //TODO: remove task argument and get task via pipe. (use state pattern?)
    /* TODO: workerMain shouldn't get any arguments except its read pipe. */
    //TODO: Dynamic task assignment for workers - at the moment ,
    Worker() : pipe(), currentTask(nullptr), idle(true), terminated(false),
               workerThread(std::thread(&Worker::mainMethod, this)) {}

    void cleanThread() {
        terminate();
        workerThread.join();
    }

    void mainMethod();

    void idleLoop();
    void runningLoop();

    void assignTask(Task *newTask);
    Task* getCurrentTask() { return currentTask; } /* TEST */
    bool isIdle() { return idle; } /* TEST */
    void terminate();

    PipeWriter &getPipeWriter(){
        return pipe.writer();
    }

    Lock::User getLockUser(){
        return &getPipeWriter();
    }

private:
    /* TODO: make return type to unique_ptr */
    Message *waitAndGetMessage();
    std::thread workerThread;
    Pipe pipe;
    Task *currentTask;
    bool idle;
    bool terminated;
};

#endif //EVENT_MANAGER_WORKER_HPP

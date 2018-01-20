#ifndef EVENT_MANAGER_WORKER_HPP
#define EVENT_MANAGER_WORKER_HPP

#include <thread>
#include "Task.hpp"
#include "Event.hpp"
#include "Lock.hpp"

class Worker {
public:
    //TODO: Dynamic task assignment for workers - at the moment ,
    Worker() : pipe(), currentTask(nullptr), idle(true), terminated(false),
               workerThread(std::thread(&Worker::mainMethod, this)) {}

    void mainMethod();

    void assignTask(Task *newTask){
        if(idle)
            currentTask = newTask;
        else
            throw TooBusy();
    }

    /* send a message to this worker */
    void sendMessage(const Message& msg) {
        pipe.writer().writeOneMessage(msg);
    }

    Lock::User getLockUser(){
        return &getPipeWriter();
    }

    bool isIdle() const { return idle; }

    void cleanThread() {
        terminate();
        workerThread.join();
    }

    class StopRunning : public std::exception {
    public:
        const char* what() const noexcept override {return "Worker stops the current task.\n";}
    };

    class TooBusy : public std::exception {
    public:
        const char* what() const noexcept override {return "Worker is already running a task.\n";}
    };

    /* for testing */
    Task *getCurrentTask() const { return currentTask; }

    PipeWriter &getPipeWriter(){ /* TODO: remove */
        return pipe.writer();
    }

private:
    Worker(const Worker& w) = delete;
    Worker(Worker&& w) = delete;

    void idleLoop();
    void runningLoop();
    Message *waitAndGetMessage(); /* TODO: make return type to unique_ptr */
    void terminate();

    std::thread workerThread;
    Pipe pipe;
    Task *currentTask; /* TODO: remove */
    bool idle;
    bool terminated;
};

#endif //EVENT_MANAGER_WORKER_HPP

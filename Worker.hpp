#ifndef EVENT_MANAGER_WORKER_HPP
#define EVENT_MANAGER_WORKER_HPP

#include <thread>
#include <atomic>
#include "Task.hpp"
#include "Event.hpp"
#include "Lock.hpp"

class Worker {
public:
    //TODO: Dynamic task assignment for workers - at the moment ,
    Worker() : pipe(), currentTask(nullptr), idle(true),
               workerThread(std::thread(&Worker::mainMethod, this)) {}
    ~Worker();

    void mainMethod();

    void assignTask(Task *newTask) {
        if(isIdle()) {
            /* TODO: avoid casting of newTask */
            Message taskAddressMessage(NEW_TASK_MESSAGE_ID, sizeof(Task *), (char *)&newTask);
            sendMessage(taskAddressMessage);
        }
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

    bool isIdle() {
        std::lock_guard<std::mutex> guard(stateLock);
        return idle;
    }

    void cleanThread() {
        terminate();
        workerThread.join();
    }

    struct StopRunning : public std::exception {
        const char* what() const noexcept override {return "Worker stops the current task.\n";}
    };

    struct TooBusy : public std::exception {
        const char* what() const noexcept override {return "Worker is already running a task.\n";}
    };

    class DeleteNotJoinedThread : public std::exception {
        const char* what() const noexcept override {return "Worker is deleted without calling cleanThread.\n";}
    };

    /* for testing */
    Task *getCurrentTask() {
        std::lock_guard<std::mutex> guard(stateLock);
        return currentTask;
    }

    PipeWriter &getPipeWriter(){ /* TODO: remove */
        return pipe.writer();
    }

private:
    Worker(const Worker& w) = delete;
    Worker(Worker&& w) = delete;

    static constexpr Message::ID NEW_TASK_MESSAGE_ID = 0; /* TODO: define proper Message ID */
    static constexpr Message::ID TERMINATE_WORKER = 1;

    void idleLoop();
    void runningLoop();
    Message *waitAndGetMessage(); /* TODO: make return type to unique_ptr */
    void terminate();

    std::thread workerThread;
    Pipe pipe;

    std::mutex stateLock;
    Task *currentTask; /* TODO: remove */
    bool idle;

    void setToIdleStatus();

    void setToRunningStatus(Task *newTask);
};

#endif //EVENT_MANAGER_WORKER_HPP

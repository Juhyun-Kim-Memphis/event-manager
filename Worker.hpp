#ifndef EVENT_MANAGER_WORKER_HPP
#define EVENT_MANAGER_WORKER_HPP

#include <thread>
#include <atomic>
#include "Task.hpp"
#include "Event.hpp"
#include "Lock.hpp"
#include "MessageList.hpp"

class Worker {
public:
    Worker() : pipe(), currentTask(nullptr), idle(true),
               workerThread() {
        workerThread = std::thread(&Worker::mainMethod, this);
    }

    virtual ~Worker();

    void mainMethod();

    void assignTask(Task *newTask);

    /* send a message to this worker */
    void sendMessage(const Message& msg) {
        pipe.writer().writeOneMessage(msg);
    }

    Lock::User getLockUser(){
        return &getPipeWriter();
    }

    /* TODO remove */
    bool isIdle() const {
        // std::lock_guard<std::mutex> guard(stateLock);
        return idle.load();
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

    /* for testing
     * TODO: remove (cthr should send query for status and currentTask of worker. or have its own status data for all wthrs. )
     * */
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

    static constexpr Message::ID NEW_TASK = GlobalUnique::NEW_TASK; /* TODO: define proper Message ID */
    static constexpr Message::ID TERMINATE_WORKER = GlobalUnique::TERMINATE_WORKER;

    void idleLoop();
    void runningLoop();
    Message *waitAndGetMessage(); /* TODO: make return type to unique_ptr */
    void terminate();

    std::thread workerThread;
    Pipe pipe;

    std::mutex stateLock;
    Task *currentTask; /* TODO: remove (move to Running status) */
    std::atomic<bool> idle;

    void setToIdleStatus();
    void setToRunningStatus(Task *newTask);
};

#endif //EVENT_MANAGER_WORKER_HPP

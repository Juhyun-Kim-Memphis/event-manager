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
    Worker(PipeReader &pr, Task *t) : pipeReader(pr), currentTask(t), idle(true), startDone(false), terminated(false) {}
    Worker(PipeReader &pr) : pipeReader(pr), currentTask(nullptr), idle(true), startDone(false), terminated(false), workerThread(std::thread(&Worker::mainMethod, this)) {}

/*
 * TODO: 생성자에서 Thread 정리
    ~Worker() {
        workerThread.join();
        그 외 flag들 정리
    }
*/

    void cleanThread() {
        workerThread.join();
    }

    void mainMethod() {
        try {
            tryLoop();
        } catch (const StopTask& stopTaskException) {
            /*std::cerr << "exception caught: " << stopTaskException.what() << std::endl;*/
            return;
        }
    }

    void tryLoop() {
        while (idle){
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if(currentTask){
                idle = false;
                break;
            }
            else if (terminated)
                return;
        }

        startDone = true;

        currentTask->start();

        while( !currentTask->hasQuit() ) {
            Message *msg = waitAndGetMessage();
            currentTask->handle(msg);
        }

        idle = true;
        currentTask = nullptr;
        startDone = true;
    }

    void assignTask(Task *newTask) { currentTask = newTask; }
    Task* getCurrentTask() { return currentTask; } /* TEST */
    bool isIdle() { return idle; } /* TEST */
    void terminate(PipeWriter &pw) { /* TODO: remove PipeWriter Parameter */
        terminated = true;
        if(!idle){
            char a = 'a';
            Message message(0, 1, &a);
            pw.writeOneMessage(message);
        }
        idle = true;
        currentTask = nullptr;
    }


    bool startDone; /* TEST */

private:
    /* TODO: make return type to unique_ptr */
    Message *waitAndGetMessage() {
        try{
            Message *msg = pipeReader.readOneMessage();
            if(terminated)
                throw StopTask();
            return msg;
        } catch (const StopTask& stopTaskException) {
            throw; /* rethrow StopTask */
        } catch (const std::exception& ex) {
            std::cerr<<"Exception at waitAndGetMessage: "<<ex.what()<<std::endl;
            exit(1);
        }
    }

    std::thread workerThread;

    /*TODO
    Pipe pipe*/
    PipeReader &pipeReader;
    Task *currentTask;
    bool idle;
    bool terminated;
};

#endif //EVENT_MANAGER_WORKER_HPP

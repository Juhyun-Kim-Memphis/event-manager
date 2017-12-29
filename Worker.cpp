#include "Worker.hpp"


void Worker::assignTask(Task *newTask) {
    currentTask = newTask;
}

void Worker::idleLoop() {
    while (idle){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if(currentTask){
            idle = false;
            break;
        }
        else if (terminated)
            throw StopTask();
    }
}

void Worker::runningLoop() {
    currentTask->start();

    while( !currentTask->hasQuit() ) {
        Message *msg = waitAndGetMessage();
        currentTask->handle(msg);
    }
}

void Worker::tryLoop() {

        idleLoop();
        runningLoop();
        idle = true;
        currentTask = nullptr;

}

void Worker::terminate() { /* TODO: remove PipeWriter Parameter */
    terminated = true;
    if(!idle){
        char a = 'a';
        Message message(0, 1, &a);
        pipe.writer().writeOneMessage(message);
    }
    idle = true;
//    currentTask = nullptr;
}

Message *Worker::waitAndGetMessage() {
    try{
        Message *msg = pipe.reader().readOneMessage();
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

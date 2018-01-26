#include "Worker.hpp"

void Worker::mainMethod() {
    try {
        while(true){
            if(isIdle())
                idleLoop();
            else
                runningLoop();
        }
    } catch (const StopRunning& stopTaskException) {
        /*std::cerr << "exception caught: " << stopTaskException.what() << std::endl;*/
        return;
    }
}

void Worker::idleLoop() {
    Message *rawPtr = pipe.reader().readOneMessage();
    std::unique_ptr<Message> newTaskMessage(rawPtr);

    if (newTaskMessage->getID() == TERMINATE_WORKER ){
        throw StopRunning();
    } else if(newTaskMessage->getID() == NEW_TASK ) {
        Task *newTask = *(reinterpret_cast<Task * const *>(newTaskMessage->getPayload())); /* TODO: remove cast. just send empty msg! */
        setToRunningStatus(newTask);
    } else {
        throw StopRunning();
    }

}

void Worker::runningLoop() {
    currentTask->start();

    while( !currentTask->hasQuit() ) {
        std::shared_ptr<Message> msg = waitAndGetMessage();
        currentTask->handle(msg.get());
    }

    setToIdleStatus();
}

std::shared_ptr<Message> Worker::waitAndGetMessage() {
    try{
        Message *msg_ptr = pipe.reader().readOneMessage();
        std::shared_ptr<Message> msg(msg_ptr);

        if(msg->getID() == TERMINATE_WORKER)
            throw StopRunning(); /* TODO: delete msg */

        return msg;
    } catch (const StopRunning& stopTaskException) {
        throw; /* rethrow StopRunning */
    } catch (const std::exception& ex) {
        std::cerr<<"Exception at waitAndGetMessage: "<<ex.what()<<std::endl;
        exit(1);
    }
}

/* Control thread may call this method. */
void Worker::assignTask(Task *newTask) {
    /* TODO: remove isIdle query by making CTHR. make isIdle private and remove statusLock */
    if(isIdle()) {
        /* TODO: how to avoid casting of newTask */
        Task *paylaod = newTask;
        Message newTaskMsg = Message::makeMessage(NEW_TASK, reinterpret_cast<char *>(&paylaod), sizeof(Task *));
        sendMessage(newTaskMsg);
    }
    else
        throw TooBusy();
}

void Worker::setToIdleStatus() {
    std::lock_guard<std::mutex> guard(stateLock);
    /*TODO: make this two store ordered perfectly. (i.e. currentTask is NULL if idle == true) */
    currentTask = nullptr;
    idle = true; /* Status changes to Idle */
}

void Worker::setToRunningStatus(Task *newTask) {
    std::lock_guard<std::mutex> guard(stateLock);
    currentTask = newTask;
    idle = false;
}

void Worker::terminate() { /* TODO: remove PipeWriter Parameter */
    Message terminateMessage = Message::makeDummyMessage(TERMINATE_WORKER);
    sendMessage(terminateMessage);
}

Worker::~Worker() {
    if(workerThread.joinable())
        cleanThread();
}

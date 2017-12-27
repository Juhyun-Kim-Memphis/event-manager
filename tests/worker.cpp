#include "gtest/gtest.h"
#include "../Worker.hpp"
#include <chrono>
#include <thread>

TEST(Worker, testIdle) {
    Pipe pipe;
    Worker worker(pipe.reader());
    std::thread workerThread(&Worker::mainMethod, &worker);
    EXPECT_EQ(true, worker.isIdle());
    worker.terminate(pipe.writer());
    workerThread.join();
}

class DummyTask : public Task {
public:
    DummyTask() {}
    void start() override {
        /* DO NOTHING */
    }

    void handle(Message *msg) override {
        quit();
    }
};

TEST(Worker, testPassTaskToWorker) {
    Pipe pipe;
    Worker worker(pipe.reader());
    std::thread workerThread(&Worker::mainMethod, &worker);
    DummyTask task;
    worker.assignTask(&task);

    while( !worker.startDone )
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); /* BUSY WAITING */

    EXPECT_EQ(&task, worker.getCurrentTask());
    EXPECT_FALSE(worker.isIdle());
    worker.terminate(pipe.writer());
    workerThread.join();

    EXPECT_EQ(nullptr, worker.getCurrentTask());
    EXPECT_TRUE(worker.isIdle());
}

TEST(Worker, testPassTaskToWorkerAndBackToIdleState) {
    Pipe pipe;
    Worker worker(pipe.reader());
    std::thread workerThread(&Worker::mainMethod, &worker);
    DummyTask task;
    worker.assignTask(&task);

    while( !worker.startDone )
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); /* BUSY WAITING */
    worker.startDone=false;

    Message dummyMsg = Message::makeDummyMessage();
    pipe.writer().writeOneMessage(dummyMsg);

    while( !worker.startDone )
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); /* BUSY WAITING */

    EXPECT_EQ(nullptr, worker.getCurrentTask());
    EXPECT_TRUE(worker.isIdle());
    workerThread.join();
}

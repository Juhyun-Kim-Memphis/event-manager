#include "gtest/gtest.h"
#include "../Worker.hpp"
#include <chrono>

TEST(Worker, testIdle) {
    Pipe pipe;
    Worker worker(pipe.reader());
    EXPECT_EQ(true, worker.isIdle());
    worker.terminate(pipe.writer());
    worker.cleanThread();
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
    DummyTask task;
    worker.assignTask(&task);

    while( !worker.startDone )
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); /* BUSY WAITING */

    EXPECT_EQ(&task, worker.getCurrentTask());
    EXPECT_FALSE(worker.isIdle());
    worker.terminate(pipe.writer());
    worker.cleanThread();

    EXPECT_EQ(nullptr, worker.getCurrentTask());
    EXPECT_TRUE(worker.isIdle());
}

TEST(Worker, testPassTaskToWorkerAndBackToIdleState) {
    Pipe pipe;
    Worker worker(pipe.reader());
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
    worker.cleanThread();
}
#include "gtest/gtest.h"
#include "../Worker.hpp"
#include <chrono>

TEST(Worker, testIdle) {
    Worker worker;
    EXPECT_EQ(true, worker.isIdle());
    worker.terminate();
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
    Worker worker;
    DummyTask task;
    worker.assignTask(&task);

    std::this_thread::sleep_for(std::chrono::milliseconds(500)); /* BUSY WAITING */

    EXPECT_EQ(&task, worker.getCurrentTask());
    EXPECT_FALSE(worker.isIdle());
    worker.terminate();
    worker.cleanThread();

    EXPECT_EQ(nullptr, worker.getCurrentTask());
    EXPECT_TRUE(worker.isIdle());
}

TEST(Worker, testPassTaskToWorkerAndBackToIdleState) {
    Worker worker;
    DummyTask task;
    worker.assignTask(&task);

    std::this_thread::sleep_for(std::chrono::milliseconds(500)); /* BUSY WAITING */

    Message dummyMsg = Message::makeDummyMessage();
    worker.getPipeWriter().writeOneMessage(dummyMsg);

    std::this_thread::sleep_for(std::chrono::milliseconds(500)); /* BUSY WAITING */

    EXPECT_EQ(nullptr, worker.getCurrentTask());
    EXPECT_TRUE(worker.isIdle());
    worker.cleanThread();
}
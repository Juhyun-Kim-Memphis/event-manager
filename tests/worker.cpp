#include "gtest/gtest.h"
#include "../Worker.hpp"
#include "test_utils.hpp"

TEST(Worker, testIdle) {
    Worker worker;
    EXPECT_EQ(true, worker.isIdle());
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

    EXPECT_TRUE(WAIT_FOR_EQ(false, worker.isIdle()));
    EXPECT_EQ(&task, worker.getCurrentTask());
    worker.cleanThread();
}

TEST(Worker, testAssignTaskToRunningWorker) {
    Worker worker;
    DummyTask task;
    worker.assignTask(&task);

    EXPECT_TRUE(WAIT_FOR_EQ(false, worker.isIdle())); /* need to wait until worker gets running. */
    EXPECT_THROW(worker.assignTask(&task), Worker::TooBusy);
    worker.cleanThread();
}

TEST(Worker, testPassTaskToWorkerAndBackToIdleState) {
    Worker worker;
    DummyTask task;
    worker.assignTask(&task);

    Message dummyMsg = Message::makeDummyMessage();
    worker.sendMessage(dummyMsg);

    EXPECT_TRUE(WAIT_FOR_EQ(true, worker.isIdle()));
    EXPECT_TRUE(WAIT_FOR_EQ(nullptr, worker.getCurrentTask()));
    worker.cleanThread();
}

TEST(Worker, testPassTaskToWorkerAndBackToIdleStateAndPassTaskAndBackToIdleState) {
    Worker worker;
    DummyTask task, anotherTask;
    worker.assignTask(&task);

    Message dummyMsg = Message::makeDummyMessage();
    worker.sendMessage(dummyMsg);

    EXPECT_TRUE(WAIT_FOR_EQ(true, worker.isIdle()));

    worker.assignTask(&anotherTask);

    EXPECT_TRUE(WAIT_FOR_EQ(false, worker.isIdle()));
    EXPECT_EQ(&anotherTask, worker.getCurrentTask());

    worker.sendMessage(dummyMsg);

    EXPECT_TRUE(WAIT_FOR_EQ(true, worker.isIdle()));

    worker.cleanThread();
}
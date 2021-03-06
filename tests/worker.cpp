#include "gtest/gtest.h"
#include "../Worker.hpp"
#include "test_utils.hpp"

TEST(Worker, testIdle) {
    Worker worker;
    EXPECT_EQ(true, worker.isIdle());
    worker.cleanThread();
}

TEST(Worker, testWorkerDeletion) {
    auto createWorker = [](){
        Worker worker;
    };
    /* worker destructor will be called. */
    EXPECT_NO_THROW(createWorker());
}

class DummyTask : public Task {
public:
    DummyTask() {}
    void start() override {
        /* DO NOTHING. */
    }

    void handle(Message *msg) override {
        /* this task finishes when receiving a message.  */
        quit();
    }
};

TEST(Worker, testPassTaskToWorker) {
    Worker worker;
    DummyTask task;
    worker.assignTask(&task);

    EXPECT_TRUE(WAIT_FOR_EQ<bool>(false, std::bind(&Worker::isIdle, &worker)));
    EXPECT_EQ(&task, worker.getCurrentTask());
    worker.cleanThread();
}

TEST(Worker, testAssignTaskToRunningWorker) {
    Worker worker;
    DummyTask task;
    worker.assignTask(&task);

    EXPECT_TRUE(WAIT_FOR_EQ<bool>(false, std::bind(&Worker::isIdle, &worker))); /* need to wait until worker gets running. */
    EXPECT_THROW(worker.assignTask(&task), Worker::TooBusy);
    worker.cleanThread();
}

TEST(Worker, testPassTaskToWorkerAndBackToIdleState) {
    Worker worker;
    DummyTask task;
    worker.assignTask(&task);

    Message dummyMsg = Message::makeDummyMessage();
    worker.sendMessage(dummyMsg);

    EXPECT_TRUE(WAIT_FOR_EQ<bool>(true, std::bind(&Worker::isIdle, &worker)));
    EXPECT_TRUE(WAIT_FOR_EQ<Task *>(nullptr, std::bind(&Worker::getCurrentTask, &worker)));
    EXPECT_EQ(nullptr, worker.getCurrentTask());
    worker.cleanThread();
}

TEST(Worker, testPassTaskToWorkerAndBackToIdleStateAndPassTaskAndBackToIdleState) {
    Worker worker;
    DummyTask task, anotherTask;
    worker.assignTask(&task);

    Message dummyMsg = Message::makeDummyMessage();
    worker.sendMessage(dummyMsg);

    EXPECT_TRUE(WAIT_FOR_EQ<bool>(true, std::bind(&Worker::isIdle, &worker)));

    worker.assignTask(&anotherTask);

    EXPECT_TRUE(WAIT_FOR_EQ<bool>(false, std::bind(&Worker::isIdle, &worker)));

    /* TODO: how to remove sleep? getCurrentTask gives task, not  anotherTask.. (use atomic?) */
    EXPECT_EQ(&anotherTask, worker.getCurrentTask());

    worker.sendMessage(dummyMsg);

    EXPECT_TRUE(WAIT_FOR_EQ<bool>(true, std::bind(&Worker::isIdle, &worker)));

    worker.cleanThread();
}
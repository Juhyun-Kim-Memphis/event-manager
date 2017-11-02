#include <thread>
#include "gtest/gtest.h"
#include "../Task.hpp"
#include "../Worker.hpp"

//TEST(BasicTest, testModuleChangeItsVariable) {
//    Pipe pipe;
//    Module module(0);
//    // set 1 to Module's shared variable.
//    ModifyTask task(pipe.getWritefd(), 1, module);
//
//    EXPECT_EQ(0, module.getSharedVar());
//    //TODO: workerMain shouldn't get any arguments except its read pipe.
//    std::thread worker(&Worker::mainLoop, Worker(pipe.getReadfd(), &task));
//
//    worker.join();
//    EXPECT_EQ(1, module.getSharedVar());
//}
//
//TEST(BasicTest, testFailToAcquireLock) {
//    Pipe pipe[2];
//
//    Module module(0);
//    ModifyTask modifyTask(pipe[0].getWritefd(), 1, module);
//    LockReleasingTask taskHavingLock(pipe[1].getWritefd(), module);
//
//    EXPECT_EQ(0, module.getSharedVar());
//
//    // sleep 1 second and release lock
//    std::thread worker2(&Worker::mainLoop, Worker(pipe[1].getReadfd(), &taskHavingLock));
//    EXPECT_EQ(0, module.getSharedVar());
//    std::thread worker1(&Worker::mainLoop, Worker(pipe[0].getReadfd(), &modifyTask));
//    EXPECT_EQ(0, module.getSharedVar());
//
//    worker1.join();
//    worker2.join();
//
//    EXPECT_EQ(1, module.getSharedVar());
//}

TEST(BasicTest, testMultiplexing) {
    Pipe pipe[2];

    ModuleHavingTwoSharedVar module(0, 0);
    LockAcquireTask incrementTask(pipe[0].getWritefd(), module);
    MultiLockWaitTask multiLockWaitTask(pipe[1].getWritefd(), module);

    std::thread lockOwner(&Worker::mainLoop, Worker(pipe[0].getReadfd(), &incrementTask));

    lockOwner.join();

    vector<Lock *> locksExpected;
    locksExpected.push_back(&module.lockA);
    locksExpected.push_back(&module.lockB);
    EXPECT_EQ(locksExpected, incrementTask.getLocks());

    std::thread waiter(&Worker::mainLoop, Worker(pipe[1].getReadfd(), &multiLockWaitTask));
    waiter.join();

    vector<Lock *> waitLocksExpected;
    waitLocksExpected.push_back(&module.lockA);
    waitLocksExpected.push_back(&module.lockB);
    EXPECT_EQ(waitLocksExpected, multiLockWaitTask.testWaitingLock());
}

//TEST(BasicTest, testMultiplexingAndRelease) {
//    Pipe pipe[2];
//
//    ModuleHavingTwoSharedVar module(0, 0);
//    LockAcquireTask incrementTask(pipe[0].getWritefd(), module);
//    MultiLockWaitTask multiLockWaitTask(pipe[1].getWritefd(), module);
//
//    std::thread lockOwner(&Worker::mainLoop, Worker(pipe[0].getReadfd(), &incrementTask));
//
//    std::thread waiter(&Worker::mainLoop, Worker(pipe[1].getReadfd(), &multiLockWaitTask));
//
//    waiter.join();
//    lockOwner.join();
//
//    EXPECT_EQ(string("waiting: lock Alock B"), multiLockWaitTask.testWaitingLock());
//}
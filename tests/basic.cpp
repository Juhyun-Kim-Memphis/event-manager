#include <thread>
#include "gtest/gtest.h"
#include "../Task.hpp"
#include "../Worker.hpp"

TEST(TaskAndEvent, testModuleChangeItsVariable) {
    Pipe pipe;
    Module module(0);
    // set 1 to Module's shared variable.
    ModifyTask task(pipe.getWritefd(), 1, module);

    EXPECT_EQ(0, module.getSharedVar());
    //TODO: workerMain shouldn't get any arguments except its read pipe.
    std::thread worker(&Worker::mainLoop, Worker(pipe.getReadfd(), &task));

    worker.join();
    EXPECT_EQ(1, module.getSharedVar());
}

TEST(TaskAndEvent, testFailToAcquireLock) {
    Pipe pipe[2];

    Module module(0);
    ModifyTask modifyTask(pipe[0].getWritefd(), 1, module);
    LockReleasingTask taskHavingLock(pipe[1].getWritefd(), module);

    cout << "Task1:"<<pipe[0].getWritefd() << ", Task2:"<<pipe[1].getWritefd() <<"\n";
    EXPECT_EQ(0, module.getSharedVar());

    // sleep 1 second and release lock
    std::thread worker2(&Worker::mainLoop, Worker(pipe[1].getReadfd(), &taskHavingLock));
    EXPECT_EQ(0, module.getSharedVar());
    usleep(300);
    std::thread worker1(&Worker::mainLoop, Worker(pipe[0].getReadfd(), &modifyTask));

    worker1.join();
    worker2.join();

    EXPECT_EQ(1, module.getSharedVar());
}

TEST(TaskAndEvent, testWaitMultipleLocks) {
    Pipe pipe[2];
    Lock lockA, lockB;
    LockAcquireTask acquireTask(pipe[0].getWritefd(), &lockA, &lockB);
    LockAcquireTask multiLockWaitTask(pipe[1].getWritefd(), &lockA, &lockB);

    std::thread lockOwner(&Worker::mainLoop, Worker(pipe[0].getReadfd(), &acquireTask));
    lockOwner.join();
    EXPECT_EQ(vector<Lock *>({&lockA, &lockB}), acquireTask.getAcquiredLocks());

    std::thread waiter(&Worker::mainLoop, Worker(pipe[1].getReadfd(), &multiLockWaitTask));
    waiter.join();
    EXPECT_EQ(vector<Lock *>({&lockA, &lockB}), multiLockWaitTask.getAwaitedLocks());
}


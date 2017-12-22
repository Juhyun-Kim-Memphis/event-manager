#include <thread>
#include <ostream>
#include <fstream>
#include "gtest/gtest.h"
#include "../Task.hpp"
#include "../Worker.hpp"


TEST(TaskAndEvent, testSuccessToAcquireLock) {
    Pipe pipe;
    Lock lock(0);
    LockAcquireTask task(pipe.writer(), vector<Lock*>({&lock}));

    std::thread worker(&Worker::mainLoop, Worker(pipe.reader(), &task));
    worker.join();

    EXPECT_EQ(&pipe.writer(), lock.getOwner());
    /* TODO: modify &pipe.writer() to task.getWorker or something other.. */
    EXPECT_EQ(true, lock.hasLocked());
    EXPECT_EQ(vector<Lock *>({&lock}), task.getAcquiredLocks());
    EXPECT_EQ(vector<Lock *>(), task.getAwaitedLocks());
}

TEST(TaskAndEvent, testFailToAcquireLock) {
    Pipe dummyPipe;
    Lock::User dummyPlayer = &dummyPipe.writer();
    Lock lock(0);

    lock.acquire(dummyPlayer);
    EXPECT_EQ(true, lock.hasLocked());
    EXPECT_EQ(dummyPlayer, lock.getOwner());

    Pipe pipe;
    LockAcquireTask task(pipe.writer(), vector<Lock*>({&lock}));
    std::thread worker(&Worker::mainLoop, Worker(pipe.reader(), &task));
    worker.join();

    EXPECT_EQ(true, lock.isInWaiters(&pipe.writer()));
    EXPECT_EQ(vector<Lock *>(), task.getAcquiredLocks());
    EXPECT_EQ(vector<Lock *>({&lock}), task.getAwaitedLocks());
}

TEST(TaskAndEvent, testWaitMultipleLocks) {
    Pipe pipe[2];
    Lock lockA(1), lockB(2);
    vector<Lock*> targetLocks({&lockA, &lockB});
    LockAcquireTask acquireTask(pipe[0].writer(), targetLocks);
    LockAcquireTask multiLockWaitTask(pipe[1].writer(), targetLocks);

    std::thread lockOwner(&Worker::mainLoop, Worker(pipe[0].reader(), &acquireTask));
    lockOwner.join();
    EXPECT_EQ(vector<Lock*>({&lockA, &lockB}), acquireTask.getAcquiredLocks());

    std::thread waiter(&Worker::mainLoop, Worker(pipe[1].reader(), &multiLockWaitTask));
    waiter.join();
    EXPECT_EQ(vector<Lock*>({&lockA, &lockB}), multiLockWaitTask.getAwaitedLocks());
}

/* TODO: workerMain shouldn't get any arguments except its read pipe. */
/* TODO: make function to force to quit the task */

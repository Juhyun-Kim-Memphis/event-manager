#include <thread>
#include <ostream>
#include <fstream>
#include "gtest/gtest.h"
#include "../Task.hpp"
#include "../Worker.hpp"

TEST(TaskAndEvent, testLockUserAndPipe) {
    Pipe pipe;
    Pipe pipeForReceiver;
    LockUser sender(pipe.getWritefd());
    LockUser receiver(pipeForReceiver.getWritefd());
    Event event('k');
    char buf[100];

    sender.sendEvent(receiver, event);
    read(pipeForReceiver.getReadfd(), buf, 1);

    EXPECT_EQ('k', buf[0]);
}

TEST(TaskAndEvent, testSuccessToAcquireLock) {
    Pipe pipe;
    Lock lock(0);
    LockAcquireTask task(pipe.getWritefd(), vector<Lock*>({&lock}));

    std::thread worker(&Worker::mainLoop, Worker(pipe.getReadfd(), &task));
    worker.join();

    EXPECT_EQ(task.getWritePipeFd(), lock.getOwner());
    EXPECT_EQ(true, lock.hasLocked());
    EXPECT_EQ(vector<Lock *>({&lock}), task.getAcquiredLocks());
    EXPECT_EQ(vector<Lock *>(), task.getAwaitedLocks());
}

TEST(TaskAndEvent, testFailToAcquireLock) {
    int dummyFd = 0;
    Pipe pipe;
    Lock lock(0);
    LockAcquireTask task(pipe.getWritefd(), vector<Lock*>({&lock}));

    lock.acquire(dummyFd);
    EXPECT_EQ(true, lock.hasLocked());
    EXPECT_EQ(dummyFd, lock.getOwner());

    std::thread worker(&Worker::mainLoop, Worker(pipe.getReadfd(), &task));
    worker.join();

    EXPECT_EQ(true, lock.isInWaiters(pipe.getWritefd()));
    EXPECT_EQ(vector<Lock *>(), task.getAcquiredLocks());
    EXPECT_EQ(vector<Lock *>({&lock}), task.getAwaitedLocks());
}

TEST(TaskAndEvent, testWaitMultipleLocks) {
    Pipe pipe[2];
    Lock lockA(1), lockB(2);
    vector<Lock*> targetLocks({&lockA, &lockB});
    LockAcquireTask acquireTask(pipe[0].getWritefd(), targetLocks);
    LockAcquireTask multiLockWaitTask(pipe[1].getWritefd(), targetLocks);

    std::thread lockOwner(&Worker::mainLoop, Worker(pipe[0].getReadfd(), &acquireTask));
    lockOwner.join();
    EXPECT_EQ(vector<Lock*>({&lockA, &lockB}), acquireTask.getAcquiredLocks());

    std::thread waiter(&Worker::mainLoop, Worker(pipe[1].getReadfd(), &multiLockWaitTask));
    waiter.join();
    EXPECT_EQ(vector<Lock*>({&lockA, &lockB}), multiLockWaitTask.getAwaitedLocks());
}

/* TODO: workerMain shouldn't get any arguments except its read pipe. */
/* TODO: make function to force to quit the task */

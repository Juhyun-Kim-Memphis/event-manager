#include <chrono>
#include <thread>
#include <ostream>
#include <fstream>
#include "gtest/gtest.h"
#include "../Task.hpp"
#include "../Worker.hpp"
#include "test_utils.hpp"

/* TODO: move or remove all worker here. Worker testcases should go to tests/worker.cpp
 * */

class LockAcquireTask : public Task {
public:
    LockAcquireTask(PipeWriter &pw, vector<Lock*> requiredLocks)
            : Task(), lockUser(pw), requiredLocks(std::move(requiredLocks)) {}

    void start() override {
        Lock::User myself = &lockUser;

        for(Lock *lock : requiredLocks){
            if(lock->acquire(myself))
                acquiredLocks.push_back(lock);
        }
        quit();
    }

    vector<Lock*> getAwaitedLocks() {
        Lock::User myself = &lockUser;
        vector<Lock*> awaitedLocks;

        for(Lock *lock : requiredLocks){
            if(lock->isInWaiters(myself))
                awaitedLocks.push_back(lock);
        }

        return awaitedLocks;
    }

    vector<Lock*> getAcquiredLocks() {
        return acquiredLocks;
    }

    void handle(Message *msg) override {
        throw NotImplementedHandle();
    }

private:
    const vector<Lock*> requiredLocks;
    vector<Lock*> acquiredLocks;
    PipeWriter &lockUser;
};

TEST(Task, testSuccessToAcquireLock) {
    Lock lock;
    Pipe pipe;
    LockAcquireTask task(pipe.writer(), vector<Lock*>({&lock}));

    task.start();

    EXPECT_EQ(&pipe.writer(), lock.getOwner());
    EXPECT_EQ(true, lock.hasLocked());
    EXPECT_EQ(vector<Lock *>({&lock}), task.getAcquiredLocks());
    EXPECT_EQ(vector<Lock *>(), task.getAwaitedLocks());
}

TEST(Task, testFailToAcquireLock) {
    Pipe dummyPipe;
    Lock::User dummyPlayer = &dummyPipe.writer();
    Lock lock;

    /* testing thread(this thread) acquires the lock. */
    lock.acquire(dummyPlayer);
    EXPECT_EQ(true, lock.hasLocked());
    EXPECT_EQ(dummyPlayer, lock.getOwner());

    Pipe pipeForTask;
    LockAcquireTask task(pipeForTask.writer(), vector<Lock*>({&lock}));
    Lock::User taskRunner = &pipeForTask.writer();
    task.start();

    EXPECT_EQ(true, lock.isInWaiters(taskRunner));
    EXPECT_EQ(vector<Lock *>(), task.getAcquiredLocks());
    EXPECT_EQ(vector<Lock *>({&lock}), task.getAwaitedLocks());
}

TEST(Task, testWaitMultipleLocks) {
    Lock lockA, lockB;
    Pipe pipeForOwner;
    Pipe pipeForWaiter;

    vector<Lock*> targetLocks({&lockA, &lockB});
    LockAcquireTask acquireTask(pipeForOwner.writer(), targetLocks);
    LockAcquireTask multiLockWaitTask(pipeForWaiter.writer(), targetLocks);

    acquireTask.start();

    EXPECT_EQ(vector<Lock*>({&lockA, &lockB}), acquireTask.getAcquiredLocks());

    multiLockWaitTask.start();

    EXPECT_EQ(vector<Lock*>({&lockA, &lockB}), multiLockWaitTask.getAwaitedLocks());
}
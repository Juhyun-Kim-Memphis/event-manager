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

private:
    const vector<Lock*> requiredLocks;
    vector<Lock*> acquiredLocks;
    PipeWriter &lockUser;
};

TEST(TaskAndEvent, testSuccessToAcquireLock) {
    Lock lock;
    Pipe pipe;
    LockAcquireTask task(pipe.writer(), vector<Lock*>({&lock}));

    task.start();

    EXPECT_EQ(&pipe.writer(), lock.getOwner());
    EXPECT_EQ(true, lock.hasLocked());
    EXPECT_EQ(vector<Lock *>({&lock}), task.getAcquiredLocks());
    EXPECT_EQ(vector<Lock *>(), task.getAwaitedLocks());
}

TEST(TaskAndEvent, testFailToAcquireLock) {
    Pipe dummyPipe;
    Lock::User dummyPlayer = &dummyPipe.writer();
    Lock lock;

    /* testing thread(this thread) acquires the lock. */
    lock.acquire(dummyPlayer);
    EXPECT_EQ(true, lock.hasLocked());
    EXPECT_EQ(dummyPlayer, lock.getOwner());

    Worker worker;
    LockAcquireTask task(worker.getPipeWriter(), vector<Lock*>({&lock}));
    worker.assignTask(&task);

    /* wait for the task to be done. */
    WAIT_FOR_EQ<bool>(false, std::bind(&Worker::isIdle, &worker), 5);
    EXPECT_TRUE(WAIT_FOR_EQ<bool>(true, std::bind(&Worker::isIdle, &worker)));

    EXPECT_EQ(true, lock.isInWaiters(worker.getLockUser()));
    EXPECT_EQ(vector<Lock *>(), task.getAcquiredLocks());
    EXPECT_EQ(vector<Lock *>({&lock}), task.getAwaitedLocks());

    worker.cleanThread();
}

TEST(TaskAndEvent, testWaitMultipleLocks) {
    Lock lockA, lockB;
    Worker lockOwner;
    Worker waiter;

    vector<Lock*> targetLocks({&lockA, &lockB});
    LockAcquireTask acquireTask(lockOwner.getPipeWriter(), targetLocks);
    LockAcquireTask multiLockWaitTask(waiter.getPipeWriter(), targetLocks);

    lockOwner.assignTask(&acquireTask);
    /* wait for the lockOwner to be done. */
    WAIT_FOR_EQ<bool>(false, std::bind(&Worker::isIdle, &lockOwner), 5);
    EXPECT_TRUE(WAIT_FOR_EQ<bool>(true, std::bind(&Worker::isIdle, &lockOwner)));

    EXPECT_EQ(vector<Lock*>({&lockA, &lockB}), acquireTask.getAcquiredLocks());

    waiter.assignTask(&multiLockWaitTask);
    /* wait for the waiter to be done. */
    WAIT_FOR_EQ<bool>(false, std::bind(&Worker::isIdle, &waiter), 5);
    EXPECT_TRUE(WAIT_FOR_EQ<bool>(true, std::bind(&Worker::isIdle, &waiter)));

    EXPECT_EQ(vector<Lock*>({&lockA, &lockB}), multiLockWaitTask.getAwaitedLocks());

    lockOwner.cleanThread();
    waiter.cleanThread();
}
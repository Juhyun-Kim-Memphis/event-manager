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
    WAIT_FOR_EQ<bool>(false, std::bind(&Worker::isIdle, &worker));
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
    WAIT_FOR_EQ<bool>(false, std::bind(&Worker::isIdle, &lockOwner));
    EXPECT_TRUE(WAIT_FOR_EQ<bool>(true, std::bind(&Worker::isIdle, &lockOwner)));

    EXPECT_EQ(vector<Lock*>({&lockA, &lockB}), acquireTask.getAcquiredLocks());

    waiter.assignTask(&multiLockWaitTask);
    /* wait for the waiter to be done. */
    WAIT_FOR_EQ<bool>(false, std::bind(&Worker::isIdle, &waiter));
    EXPECT_TRUE(WAIT_FOR_EQ<bool>(true, std::bind(&Worker::isIdle, &waiter)));

    EXPECT_EQ(vector<Lock*>({&lockA, &lockB}), multiLockWaitTask.getAwaitedLocks());

    lockOwner.cleanThread();
    waiter.cleanThread();
}

class LockWaitingTask : public Task {
public:
    LockWaitingTask(PipeWriter &pw, Lock &lock) : Task()
            , lockUser(pw), desiredLock(lock)
            , waiting(false) {}

    void start() override {
        if(desiredLock.acquire(&lockUser)){
            throw std::string("LockWaitingTask assumes the desiredLock is already taken.");
        }
        else
            waiting = true;

    }

    void handle(Message *msg) override {
        auto *convertedEvent = LockOwnershipChange::makeFromMsg(*msg);
        if(convertedEvent->getEventID() != 'r')
            throw std::exception();
        delete msg;
        delete convertedEvent;
        quit();
    }

    bool isWaiting() { return waiting; };

private:
    Lock &desiredLock;
    PipeWriter &lockUser;
    bool waiting;
};

/* TODO: remove this.
 * handle과 start가 잘 호출되는지 확인하는 test는 worker에서 하고
 * handle에서 적절하게 eventType으로 deserialize 및 event handler 호출하는 test는 추가 해야함.
 * */
//TEST(TaskAndEvent, testEventWaitingTask) {
//    Lock lock;
//    Pipe pipeForTester;
//    Lock::User tester = &pipeForTester.writer();
//    lock.acquire(tester);
//
//    EXPECT_EQ(tester, lock.getOwner());
//
//    Worker worker;
//    LockWaitingTask task(worker.getPipeWriter(), lock);
//    worker.assignTask(&task);
//
//    std::this_thread::sleep_for(std::chrono::milliseconds(200));
//    EXPECT_EQ(true, task.isWaiting());
//    lock.release();
//
//    std::this_thread::sleep_for(std::chrono::milliseconds(200));
//    EXPECT_EQ(true, task.hasQuit());
//    worker.cleanThread();
//}

#include <chrono>
#include <thread>
#include <ostream>
#include <fstream>
#include "gtest/gtest.h"
#include "../Task.hpp"
#include "../Worker.hpp"

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
    Worker worker;
    LockAcquireTask task(worker.getPipeWriter(), vector<Lock*>({&lock}));
    worker.assignTask(&task);

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    EXPECT_EQ(worker.getLockUser(), lock.getOwner());
    /* TODO: modify &pipe.writer() to task.getWorker or something other.. */
    EXPECT_EQ(true, lock.hasLocked());
    EXPECT_EQ(vector<Lock *>({&lock}), task.getAcquiredLocks());
    EXPECT_EQ(vector<Lock *>(), task.getAwaitedLocks());

    worker.cleanThread();
}

TEST(TaskAndEvent, testFailToAcquireLock) {
    Pipe dummyPipe;
    Lock::User dummyPlayer = &dummyPipe.writer();
    Lock lock;

    lock.acquire(dummyPlayer);
    EXPECT_EQ(true, lock.hasLocked());
    EXPECT_EQ(dummyPlayer, lock.getOwner());

    Worker worker;
    LockAcquireTask task(worker.getPipeWriter(), vector<Lock*>({&lock}));
    worker.assignTask(&task);

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
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
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    EXPECT_EQ(vector<Lock*>({&lockA, &lockB}), acquireTask.getAcquiredLocks());

    waiter.assignTask(&multiLockWaitTask);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
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

TEST(TaskAndEvent, testEventWaitingTask) {
    Lock lock;
    Pipe pipeForTester;
    Lock::User tester = &pipeForTester.writer();
    lock.acquire(tester);

    EXPECT_EQ(tester, lock.getOwner());

    Worker worker;
    LockWaitingTask task(worker.getPipeWriter(), lock);
    worker.assignTask(&task);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(true, task.isWaiting());
    lock.release();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(true, task.hasQuit());
    worker.cleanThread();
}

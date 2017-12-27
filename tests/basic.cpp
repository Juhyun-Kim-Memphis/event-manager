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
    Pipe pipe;
    Lock lock;
    LockAcquireTask task(pipe.writer(), vector<Lock*>({&lock}));

    std::thread worker(&Worker::mainMethod, Worker(pipe.reader(), &task));
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
    Lock lock;

    lock.acquire(dummyPlayer);
    EXPECT_EQ(true, lock.hasLocked());
    EXPECT_EQ(dummyPlayer, lock.getOwner());

    Pipe pipe;
    LockAcquireTask task(pipe.writer(), vector<Lock*>({&lock}));
    std::thread worker(&Worker::mainMethod, Worker(pipe.reader(), &task));
    worker.join();

    EXPECT_EQ(true, lock.isInWaiters(&pipe.writer()));
    EXPECT_EQ(vector<Lock *>(), task.getAcquiredLocks());
    EXPECT_EQ(vector<Lock *>({&lock}), task.getAwaitedLocks());
}

TEST(TaskAndEvent, testWaitMultipleLocks) {
    Pipe pipe[2];
    Lock lockA, lockB;
    vector<Lock*> targetLocks({&lockA, &lockB});
    LockAcquireTask acquireTask(pipe[0].writer(), targetLocks);
    LockAcquireTask multiLockWaitTask(pipe[1].writer(), targetLocks);

    std::thread lockOwner(&Worker::mainMethod, Worker(pipe[0].reader(), &acquireTask));
    lockOwner.join();
    EXPECT_EQ(vector<Lock*>({&lockA, &lockB}), acquireTask.getAcquiredLocks());

    std::thread waiter(&Worker::mainMethod, Worker(pipe[1].reader(), &multiLockWaitTask));
    waiter.join();
    EXPECT_EQ(vector<Lock*>({&lockA, &lockB}), multiLockWaitTask.getAwaitedLocks());
}

class LockWaitingTask : public Task {
public:
    LockWaitingTask(PipeWriter &pw, Lock &lock) : Task()
            , lockUser(pw), desiredLock(lock)
            , waiting(false), startDone(false) {}

    void start() override {
        if(desiredLock.acquire(&lockUser))
            throw std::string("LockWaitingTask assumes the desiredLock is already taken.");
        else
            waiting = true;

        startDone = true;
    }

    void handle(Message *msg) override {
        auto *convertedEvent = LockOwnershipChange::makeFromMsg(*msg);
        if(convertedEvent->getEventID() != 'r')
            throw std::exception();
        delete msg;
        delete convertedEvent;
        quit();
    }

    bool isWaiting() { return waiting; }
    bool isStartDone() { return startDone; };

private:
    Lock &desiredLock;
    PipeWriter &lockUser;
    bool waiting;
    bool startDone;
};

TEST(TaskAndEvent, testEventWaitingTask) {
    Lock lock;
    Pipe pipeForTester, pipeForWorker;
    Lock::User tester = &pipeForTester.writer();
    lock.acquire(tester);

    LockWaitingTask task(pipeForWorker.writer(), lock);
    std::thread worker(&Worker::mainMethod, Worker(pipeForWorker.reader(), &task));

    while( !task.isStartDone() )
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); /* BUSY WAITING */

    EXPECT_EQ(true, task.isWaiting());
    lock.release();

    while( !task.hasQuit() )
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); /* BUSY WAITING */

    EXPECT_EQ(true, task.hasQuit());
    worker.join();
}

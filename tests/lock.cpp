#include <thread>
#include "gtest/gtest.h"
#include "../Lock.hpp"

TEST(Lock, testLock) {
    Lock lock;

    Pipe pipeForOwner;
    Lock::User owner = &pipeForOwner.writer();
    lock.acquire(owner);
    EXPECT_EQ(true, lock.hasLocked());
    EXPECT_EQ(owner, lock.getOwner());
    EXPECT_EQ(false, lock.isInWaiters(owner));

    Pipe pipeForWaiter;
    Lock::User waiter = &pipeForWaiter.writer();
    lock.acquire(waiter);
    EXPECT_EQ(true, lock.hasLocked());
    EXPECT_NE(waiter, lock.getOwner());
    EXPECT_EQ(true, lock.isInWaiters(waiter));
}

TEST(Lock, testLockRelease) {
    Lock lock;

    Pipe pipeForOwner;
    Lock::User owner = &pipeForOwner.writer();
    lock.acquire(owner);
    lock.release();
    EXPECT_EQ(false, lock.hasLocked());
    EXPECT_EQ(nullptr, lock.getOwner());
}

TEST(Lock, testLockOwnerChange) {
    Lock lock;

    Pipe pipeForOwner;
    Lock::User owner = &pipeForOwner.writer();
    lock.acquire(owner);

    Pipe pipeForWaiter;
    Lock::User waiter = &pipeForWaiter.writer();
    lock.acquire(waiter);

    lock.release();
    EXPECT_EQ(true, lock.hasLocked());
    EXPECT_EQ(waiter, lock.getOwner());

    Message *msg = pipeForWaiter.reader().readOneMessage();
    auto *ownershipChangeEvent = LockOwnershipChange::newEvent(*msg);
    EXPECT_EQ(LockOwnershipChange::getMessageID(), msg->getID());
    delete msg;
    delete ownershipChangeEvent;
}

#include <thread>
#include <ostream>
#include <fstream>
#include "gtest/gtest.h"
#include "../Task.hpp"
#include "../Worker.hpp"
#include "TestUtilities.hpp"

TEST(TaskAndEvent, testSuccessToAcquireLock) {
    Pipe pipe;
    Module module;
    // Task will change Module.
    ModifyTask task(pipe.getWritefd(), module);

    //TODO: workerMain shouldn't get any arguments except its read pipe.
    std::thread worker(&Worker::mainLoop, Worker(pipe.getReadfd(), &task));
    worker.join();
    EXPECT_EQ(true, module.hasChanged());
    EXPECT_EQ(0, task.getNumOfEventHandlerCalls());
    EXPECT_EQ(false, module.lock.isInWaiters(task.getWritePipeFd()));
}

TEST(TaskAndEvent, testFailToAcquireLock) {
    Pipe pipe;
    Module module;
    ModifyTask task(pipe.getWritefd(), module);

    /* 0 is dummy parameter for acquire */
    EXPECT_EQ(true, module.lock.acquire(0)); /* lock in advance */
    std::thread worker1(&Worker::mainLoop, Worker(pipe.getReadfd(), &task));
    task.quit(); /* force to quit the task */
    worker1.join();

    EXPECT_EQ(false, module.hasChanged());
    EXPECT_EQ(0, task.getNumOfEventHandlerCalls());
    EXPECT_EQ(true, module.lock.isInWaiters(task.getWritePipeFd()));
}

TEST(TaskAndEvent, testWaitMultipleLocks) {
    Pipe pipe[2];
    Lock lockA(1), lockB(2);
    LockAcquireTask acquireTask(pipe[0].getWritefd(), &lockA, &lockB);
    LockAcquireTask multiLockWaitTask(pipe[1].getWritefd(), &lockA, &lockB);

    std::thread lockOwner(&Worker::mainLoop, Worker(pipe[0].getReadfd(), &acquireTask));
    lockOwner.join();
    EXPECT_EQ(vector<Lock *>({&lockA, &lockB}), acquireTask.getAcquiredLocks());

    std::thread waiter(&Worker::mainLoop, Worker(pipe[1].getReadfd(), &multiLockWaitTask));
    waiter.join();
    EXPECT_EQ(vector<Lock *>({&lockA, &lockB}), multiLockWaitTask.getAwaitedLocks());
}

class EventForTest : public Event {
public:
    EventForTest (EventID id) : Event(id) {}
    EventForTest () {};

    std::string description;
    std::string string;
    char singleChar;
    double doubleData;
    int intData;
    bool boolData;

    bool operator==(const EventForTest &rhs) const {
        return static_cast<const Event &>(*this) == static_cast<const Event &>(rhs) &&
               description == rhs.description &&
               string == rhs.string &&
               singleChar == rhs.singleChar &&
               doubleData == rhs.doubleData &&
               intData == rhs.intData &&
               boolData == rhs.boolData;
    }

    bool operator!=(const EventForTest &rhs) const {
        return !(rhs == *this);
    }

    friend ostream &operator<<(ostream &os, const EventForTest &test) {
        os << "id:" << test.id << " description: " << test.description << " string: " << test.string
           << " singleChar: " << test.singleChar << " doubleData: " << test.doubleData << " intData: " << test.intData
           << " boolData: " << test.boolData;
        return os;
    }
};

TEST(TaskAndEvent, testEventSerialization) {
    Pipe pipe;
    EventForTest event('t');
    event.description.assign("Some kind of descriptions.");
    event.string.assign("Another String data here\n after new line.");
    event.singleChar = 'a';
    event.doubleData = 4.56;
    event.intData = 221;
    event.boolData = false;

    write(pipe.getWritefd(), &event, sizeof(event));

    EventForTest deserializedEvent;
    read(pipe.getReadfd(), &deserializedEvent, sizeof(deserializedEvent));

    //std::cout<< deserializedEvent << "\n";

    EXPECT_EQ(event, deserializedEvent);
}
#include <thread>
#include <ostream>
#include <fstream>
#include "gtest/gtest.h"
#include "../Task.hpp"
#include "../Worker.hpp"
#include "TestUtilities.hpp"

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

    //cout << "Task1:"<<pipe[0].getWritefd() << ", Task2:"<<pipe[1].getWritefd() <<"\n";
    EXPECT_EQ(0, module.getSharedVar());

    // sleep 1 second and release lock
    std::thread worker2(&Worker::mainLoop, Worker(pipe[1].getReadfd(), &taskHavingLock));
    EXPECT_EQ(0, module.getSharedVar());
    std::thread worker1(&Worker::mainLoop, Worker(pipe[0].getReadfd(), &modifyTask));
    TestUtil::sendTriggeringEvent(&modifyTask);

    worker1.join();
    worker2.join();

    EXPECT_EQ(1, module.getSharedVar());
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
    EventForTest event('t');
    event.description.assign("Some kind of descriptions.");
    event.string.assign("Another String data here\n after new line.");
    event.singleChar = 'a';
    event.doubleData = 4.56;
    event.intData = 221;
    event.boolData = false;

    ofstream ofs("test.ros", ios::binary);
    ofs.write((char *)&event, sizeof(event));
    ofs.close();

    EventForTest deserializedEvent;
    ifstream ifs("test.ros", ios::binary);
    ifs.read((char *)&deserializedEvent, sizeof(deserializedEvent));
    ifs.close();

    std::cout<< deserializedEvent << "\n";

    EXPECT_EQ(event, deserializedEvent);
    EXPECT_EQ(1, 1);
}
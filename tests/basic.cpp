#include "gtest/gtest.h"
#include "../Task.hpp"
#include "../Module.hpp"
#include "../Lock.hpp"
#include <thread>
#ifdef _WIN32
#include <mingw.thread.h>
#endif

TEST(BasicTest, testModuleChangeItsVariable) {
    Module module(0);
    // insert integer value 1 to Module.
    ModifyTask task(1, module);

    EXPECT_EQ(0, module.getSharedVar());
    std::thread worker1(workerMain, task); //TODO: workerMain shouldn't get any arguments.
    worker1.join();
    EXPECT_EQ(1, module.getSharedVar());
}

TEST(BasicTest, testFailToAcquireLock) {
    Module module(0);
    // insert integer value 1 to Module.
    ModifyTask task(1, module);
    LockReleasingTask taskHavingLock(module);

    EXPECT_EQ(0, module.getSharedVar());

    std::thread worker1(&LockReleasingTask::acquireLock, taskHavingLock);
    worker1.join();

    std::thread worker2(workerMain, task);
    worker2.join();
    EXPECT_EQ(0, module.getSharedVar());
}
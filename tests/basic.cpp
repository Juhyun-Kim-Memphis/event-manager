#include "gtest/gtest.h"
#include "../Task.hpp"
#include "../Module.hpp"
#include "../Lock.hpp"
#include "../Pipe.hpp"
#include <thread>
#ifdef _WIN32
    #include <mingw.thread.h>
    #include <fcntl.h>
#include <unistd.h>

#define pipe(fds) _pipe(fds,4096, _O_BINARY)
#endif

TEST(BasicTest, testModuleChangeItsVariable) {
    Module module(0);
    // insert integer value 1 to Module.
    ModifyTask task(1, module);
    int pipeFd[2];

    if (pipe(pipeFd) == -1) {
        perror("pipe");
    }
    globalPipes = new PipeContainer(pipeFd[1], -1);

    EXPECT_EQ(0, module.getSharedVar());
    std::thread worker1(workerMain, pipeFd[0], task); //TODO: workerMain shouldn't get any arguments except its read pipe.
    worker1.join();
    EXPECT_EQ(1, module.getSharedVar());
}

TEST(BasicTest, testFailToAcquireLock) {
    Module module(0);
    // insert integer value 1 to Module.
    ModifyTask modifyTask(1, module);
    LockReleasingTask taskHavingLock(module);
    int pipeFd1[2];
    int pipeFd2[2];
    if (pipe(pipeFd1) == -1) {
        perror("pipe");
    }
    if (pipe(pipeFd2) == -1) {
        perror("pipe");
    }
    globalPipes = new PipeContainer(pipeFd1[1], pipeFd2[1]);

    EXPECT_EQ(0, module.getSharedVar());

    // sleep 1 second and release lock
    std::thread worker2(workerMain, pipeFd2[0], taskHavingLock);
    EXPECT_EQ(0, module.getSharedVar());
    std::thread worker1(workerMain, pipeFd1[0], modifyTask);

    worker1.join();
    worker2.join();

    EXPECT_EQ(1, module.getSharedVar());
}
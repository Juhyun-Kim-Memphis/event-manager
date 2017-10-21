#include "gtest/gtest.h"
#include "../task.hpp"

TEST(BasicTest, test1) {
    Task task;
    EXPECT_EQ(1, 1);
    EXPECT_EQ(task.getA(), 1);
}

TEST(BasicTest, test2) {
    Task task;
    EXPECT_EQ(1, 1);
    EXPECT_EQ(task.getA(), 1);
}
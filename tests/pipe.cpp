#include <gtest/gtest.h>
#include "../pipe.hpp"

TEST(Pipe, testPipeWriteAndRead) {
    //set message to write
    int *in = new int;
    *in = 7;
    Message *message= new Message(0, 4, (char *)in);

    Pipe pipe;
    pipe.writeOneMessage(*message);
    Message *result = pipe.readOneMessage();

    EXPECT_EQ(*message, *result);
    delete result;
    delete message;
}
#include <gtest/gtest.h>
#include "../pipe.hpp"

/* (char *, int), std::stringstream, std::vector<char>
 * can be alternative impl.
 * */
class ByteBuffer {
public:
    const char *data(){ return buf.str().c_str(); }
    size_t length() { return buf.str().length(); }

    void put(const int& v) {
        buf.write((char *)&v, sizeof(int));
    }
    void put(char *data, size_t length) {
        buf.write(data, length);
    }


    std::stringstream buf;
};

TEST(Pipe, testByteBuffer) {
    ByteBuffer buf;

    int a = 1;
    int b = 4;
    char str[] = {'a', 'b', 'c', 'd', 'e', 'f'};

    buf.put(a);
    buf.put(b);
    buf.put(str, sizeof str);

    char data[18];
    memcpy(data, &a, sizeof(int));
    memcpy(data + sizeof(int), &b, sizeof(int));
    memcpy((data + sizeof(int)) + sizeof(int), &str, sizeof str);

    std::cout<<buf.buf.str()<<"\n";
    std::cout<<*(int *)buf.data()<<"\n";
    std::cout<<*(int *)(buf.data() + 4)<<"\n";

    /* goal: message ==> byteArray (typecode, len, data)
     * because of 'put' ==> use stringbuf or stringstream
     * make read function (using get?)
     * remove Header struct
     * */


    EXPECT_EQ(14, buf.length());
    EXPECT_EQ(0, memcmp(buf.data(), data, buf.length()));
    EXPECT_EQ(*(int *)buf.data(), *(int *)data);
}

TEST(Pipe, testMessage) {
    int *in = new int;
    *in = 7;
    Message *message= new Message(0, 4, (char *)in);

    int *in2 = new int;
    *in2 = 7;
    Message *message2= new Message(0, 4, (char *)in2);

    EXPECT_EQ(*message, *message2);
}

/*
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
}*/

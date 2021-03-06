#include <gtest/gtest.h>
#include <boost/serialization/vector.hpp>
#include "../Pipe.hpp"


TEST(Pipe, testMessage) {
    Pipe pipe;
    char bytes[43] = "hello. this is sample string. length is 42";
    Message msg = Message::makeMessage(777, bytes, 43);

    pipe.writer().writeOneMessage(msg);

    /// ///

    Message *receivedMsg = pipe.reader().readOneMessage();

    EXPECT_EQ(777, receivedMsg->getID());
    EXPECT_EQ(43, receivedMsg->getPayloadSize());
    EXPECT_EQ(0, memcmp(bytes, receivedMsg->getPayload(), 43));
    delete receivedMsg;
}


TEST(Pipe, testHeaderOnlyMessage) {
    Pipe pipe;
    Message msg = Message::makeDummyMessage(342);
    pipe.writer().writeOneMessage(msg);
    Message *receivedMsg = pipe.reader().readOneMessage();

    EXPECT_EQ(342, receivedMsg->getID());
    EXPECT_EQ(0, receivedMsg->getPayloadSize());
    delete receivedMsg;
}

TEST(Pipe, testSendingAddress) {
    Pipe pipe;
    const int *data = new int(777);

    Message msg = Message::makeMessage(0, reinterpret_cast<char *>(&data), sizeof(int *));
    pipe.writer().writeOneMessage(msg);
    Message *receivedMsg = pipe.reader().readOneMessage();
    const int *receiveData = *(reinterpret_cast<int * const *>(receivedMsg->getPayload()));

    EXPECT_EQ(data, receiveData);
    delete receivedMsg;
    delete data;
}

class EventA : public Event {
public:
    EventA(const std::vector<int> &intli) : intli(intli){}

    static Message::TypeID getMessageID(){
        return 65;
    }

    bool operator==(const EventA &rhs) const {
        return intli == rhs.intli;
    }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & intli;
    }

    Message *newMessage() {
        std::stringbuf buf;
        std::ostream os(&buf);
        boost::archive::binary_oarchive oa(os, boost::archive::no_header);
        oa << *this;
        return RAIIWrapperMessage::newMessageByAllocatingAndCopying(getMessageID(), buf.str().c_str(), buf.str().length());
    }

    static EventA *makeFromMsg(Message &msg){
        int eventType = -1;
        EventA *result = new EventA({});

        std::stringbuf buf;
        buf.sputn(msg.getPayload(), msg.getPayloadSize());
        std::istream is(&buf);
        boost::archive::binary_iarchive ia(is, boost::archive::no_header);
        if(msg.getID() == 65)
            ia >> *result;
        else{
            throw std::string("unknown EventType: ").append(std::to_string(msg.getID()));
        }
        /* TODO: when to free buf? */
        return result;
    }

    std::vector<int> intli;

    /* TODO: class object, ptr to derived class
     * TODO: inplace deserialization for big message*/
};

TEST(Pipe, testDerivedEventWriteAndRead) {
    EventA eventA({4, 5});
    std::shared_ptr<Message> msg(eventA.newMessage());
    Pipe pipe;
    pipe.writer().writeOneMessage(*msg);

    Message *receivedMsg = pipe.reader().readOneMessage();
    EventA *receivedEvent = EventA::makeFromMsg(*receivedMsg);

    EXPECT_EQ(eventA, *receivedEvent);

    delete receivedEvent;
    delete receivedMsg;
}

/*
 * TODO:: add fork test and pipeEnd close test.
 * */

/* TODO: Message only have unique_ptr<stringbuf> (to avoid copy)
 * in this case, stringbuf already contains type, len (8byte) in front of byte array.
 * to do that, we must know the size of the raw data before put it to stringbuf.
 * caller of Message ctor should make this stringbuf by itself.
 * DO NOT USE memcpy.
 * DO NOT CONSIDER ENDIANESS in production code! (let libc++ handle endianess.)
 * */


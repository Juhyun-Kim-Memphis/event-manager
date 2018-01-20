#include <gtest/gtest.h>
#include <boost/serialization/vector.hpp>
#include "../Pipe.hpp"


TEST(Pipe, testMessage) {
    Pipe pipe;
    char bytes[43] = "hello. this is sample string. length is 42";
    Message msg(777, 43, bytes);

    pipe.writer().writeOneMessage(msg);
    Message *receivedMsg = pipe.reader().readOneMessage();
    EXPECT_EQ(777, receivedMsg->header.type);
    EXPECT_EQ(43, receivedMsg->header.length);
    EXPECT_EQ(0, memcmp(bytes, receivedMsg->data, 43));
    delete receivedMsg;
}


TEST(Pipe, testHeaderOnlyMessage) {
    Pipe pipe;
    Message msg = Message::makeDummyMessage(342);
    pipe.writer().writeOneMessage(msg);
    Message *receivedMsg = pipe.reader().readOneMessage();

    EXPECT_EQ(342, receivedMsg->header.type);
    EXPECT_EQ(0, receivedMsg->header.length);
    delete receivedMsg;
}

class EventA : public Event {
public:
    EventA(const std::vector<int> &intli) : Event('A'), intli(intli){}

    bool operator==(const EventA &rhs) const {
        return static_cast<const Event &>(*this) == static_cast<const Event &>(rhs) &&
               intli == rhs.intli;
    }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<Event>(*this);
        ar & intli;
    }

    Message makeMessage() {
        std::stringbuf buf;
        std::ostream os(&buf);
        boost::archive::binary_oarchive oa(os, boost::archive::no_header);
        oa << *this;
        return Message(getEventID(), buf.str().length(), buf.str().c_str());
    }

    static EventA *makeFromMsg(Message &msg){
        int eventType = -1;
        EventA *result = new EventA({});

        std::stringbuf buf;
        buf.sputn(msg.data, msg.header.length);
        std::istream is(&buf);
        boost::archive::binary_iarchive ia(is, boost::archive::no_header);
        if(msg.getType() == 'A')
            ia >> *result;
        else{
            throw std::string("unknown EventType: ").append(std::to_string(msg.getType()));
        }
        /* TODO: when to free buf? */
        return result;
    }

    std::vector<int> intli;

    /* TODO: class object, ptr to derived class */
};

TEST(Pipe, testDerivedEventWriteAndRead) {
    EventA eventA({4, 5});

    Pipe pipe;
    pipe.writer().writeOneMessage(eventA.makeMessage());

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


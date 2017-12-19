#include <gtest/gtest.h>
#include <boost/serialization/vector.hpp>

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

class EventB : public Event {
public:
    EventB(int a, bool b) : Event('B'), a(a), b(b) {}

private:
    int a;
    bool b;
};

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

    std::vector<int> intli;
    /* TODO: class object, ptr to derived class */
};

EventA *makeEventAFromMsg(Message &msg) {
    int eventType = -1;
    EventA *result = new EventA({});

    std::stringbuf buf;
    buf.sputn(msg.data, msg.header.length);
    std::istream is(&buf);
    boost::archive::binary_iarchive ia(is, boost::archive::no_header);
    if(msg.getType() == 'A')
        ia >> *result;
    else{
        throw std::string("unknown EventType: ").append(std::to_string(eventType));
    }
    /* TODO: when to free buf? */
    return result;
}

void setEventAToBuf(EventA &eventA, std::stringbuf &buf){
    std::ostream os(&buf);
    boost::archive::binary_oarchive oa(os, boost::archive::no_header);
    oa << eventA.getEventID();
    oa << eventA;
}

TEST(Pipe, testDerivedEventWriteAndRead) {
    EventA eventA({4, 5});
    /* TODO: eventA ==> Message
     * Message::makeMsg(eventA,
     * */

    Pipe pipe;
    {
        std::stringbuf buf;
        setEventAToBuf(eventA, buf);
        pipe.writeOneMessage(Message(eventA.getEventID(), buf.str().length(), buf.str().c_str()));
    }

    Message *receivedMsg = pipe.readOneMessage();
    EventA *receivedEvent = makeEventAFromMsg(*receivedMsg);

//    EXPECT_EQ(eventA.intli, receivedEvent->intli);
    EXPECT_EQ(eventA, *receivedEvent);
    delete receivedEvent;
    delete receivedMsg;
}



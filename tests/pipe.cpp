#include <gtest/gtest.h>
#include <boost/serialization/vector.hpp>

#include "../Pipe.hpp"


TEST(Pipe, testPipeEnd) {

}

class EventA : public Event {
public:
    EventA(const std::vector<int> &intli) : Event('A'), intli(intli){}

    bool operator==(const EventA &rhs) const {
        return static_cast<const Event &>(*this) == static_cast<const Event &>(rhs) &&
               intli == rhs.intli;
    }

    bool operator!=(const EventA &rhs) const {
        return !(rhs == *this);
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
        {
            boost::archive::binary_oarchive oa(os, boost::archive::no_header);
            oa << *this;
        }
        std::cout<< "~~~"<<buf.str().length()<<std::endl;
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
            throw std::string("unknown EventType: ").append(std::to_string(eventType));
        }
        /* TODO: when to free buf? */
        return result;
    }

    std::vector<int> intli;
    /* TODO: class object, ptr to derived class */
};


TEST(Pipe, EventASerializeTest) {
    EventA eventA({4, 5});
    std::stringbuf buf;
    std::ostream os(&buf);
    boost::archive::binary_oarchive oa(os, boost::archive::no_header);
    oa << eventA;

    EventA receivedEventA({});
    EXPECT_NE(eventA, receivedEventA);

    std::stringbuf copiedBuf;
    copiedBuf.sputn(buf.str().c_str(), buf.str().length());
    std::istream is(&copiedBuf);
    boost::archive::binary_iarchive ia(is, boost::archive::no_header);
    ia >> receivedEventA;

    EXPECT_EQ(copiedBuf.str().length() ,buf.str().length());
    EXPECT_EQ(0, memcmp(copiedBuf.str().c_str(), buf.str().c_str(), buf.str().length()));
    EXPECT_EQ(eventA, receivedEventA);
}


TEST(Pipe, testStringbufsputn) {
    char data[512] = {'2', '3', 'a'};

    std::stringbuf buf;
    buf.sputn(data, 512);

    EXPECT_EQ(512, buf.str().length());
    EXPECT_EQ(0, memcmp(data, buf.str().c_str(), buf.str().length()));
}

TEST(Pipe, testStringbufAndSerializedEventA) {
    EventA eventA({4, 5});

    std::stringbuf buf;
    std::ostream os(&buf);
    {
        boost::archive::binary_oarchive oa(os, boost::archive::no_header);
        oa << eventA;
    }

    std::stringbuf copiedBuf;
    copiedBuf.sputn(buf.str().c_str(), buf.str().length());
    EXPECT_EQ(buf.str().length(), copiedBuf.str().length());
    EXPECT_EQ(0, memcmp(buf.str().c_str(), copiedBuf.str().c_str(), copiedBuf.str().length()));
}


TEST(Pipe, TestStringBufStrCStr) {
    EventA eventA({4, 5});
    std::stringbuf buf;
    std::ostream os(&buf);
    {
        boost::archive::binary_oarchive oa(os, boost::archive::no_header);
        oa << eventA;
    }
    char res2[512];
    memcpy(res2, buf.str().c_str(), 3);
    char *res = const_cast<char *>(buf.str().c_str());
    std::cout <<"-------\n";
    printf("%p ]\n", (void *)&(res[1]));
    printf("%p ]\n", (void *)&(res2[1]));
    printf("%p ]\n", (void *)&(buf.str().c_str()[1]));
    std::cout <<"-------\n";
    printf("%d ]\n", (uint8_t)(res[1]));
    printf("%d ]\n", (uint8_t)(res2[1]));
    printf("%d ]\n", (uint8_t)(buf.str().c_str()[1]));
    std::cout <<"-------\n";
    std::cout <<std::hex<< (uint8_t)(res[1]) << std::endl;
    std::cout <<std::hex<< (uint8_t)(res2[1]) << std::endl;
    std::cout <<std::hex<< (uint8_t)(buf.str().c_str()[1]) << std::endl;
    std::cout <<"-------\n";
    uint8_t resInt = (uint8_t)res[1];
    uint8_t resInt2 = (uint8_t)res2[1];
    uint8_t oriInt = (uint8_t)buf.str().c_str()[1];
//    EXPECT_EQ(resInt, oriInt); // this will fail in windows
    EXPECT_EQ(res[1], buf.str().c_str()[1]); //why this success?
    EXPECT_EQ(resInt2, oriInt);
}

TEST(Pipe, testDerivedEventWriteAndRead) {
    EventA eventA({1, 5});

    {
        std::stringbuf buf;
        std::ostream os(&buf);
        {
            boost::archive::binary_oarchive oa(os, boost::archive::no_header);
            oa << eventA;
        }
        std::cout << buf.str().length() <<"~~!!\n";
        std::cout << buf.str() <<"~~~~!!!!\n";

    }

    std::stringbuf buf;
    std::ostream os(&buf);
    {
        boost::archive::binary_oarchive oa(os, boost::archive::no_header);
        oa << eventA;
    }
    std::cout<< "~~~"<<buf.str().length()<<std::endl;
    Message msg(eventA.getEventID(), buf.str().length(), buf.str().c_str());
//    Message msg = eventA.makeMessage();
    std::cout << buf.str().c_str()[1] <<"..oh when~~~!!!"<<std::endl;
    std::cout << msg.data[1] <<"..when~~~!!!"<<std::endl;
    {
        EventA receivedEventA({4, 5});
        std::stringbuf copiedBuf;
        copiedBuf.sputn(msg.data, 26);
//        copiedBuf.pubseekoff(0, std::ios_base::beg);
        EXPECT_EQ(msg.header.length, copiedBuf.str().length());
        EXPECT_EQ(0, memcmp(msg.data, copiedBuf.str().c_str(), copiedBuf.str().length()));

        EXPECT_EQ(buf.str().length(), msg.header.length);
        EXPECT_EQ(0, memcmp(buf.str().c_str(), msg.data, buf.str().length()));

        std::cout << buf.str().c_str()[0] <<"..~~~~!!!!\n";
        std::cout << msg.data[0] <<"..~~~!!!"<<std::endl;
        std::cout << buf.str() <<"~~~~!!!!\n";
        std::cout << std::string(msg.data, msg.header.length) <<"~~~!!!"<<std::endl;
        std::cout << copiedBuf.str() <<"~~~!!!"<<std::endl;

        EXPECT_EQ(buf.str().length(), copiedBuf.str().length());
//        EXPECT_EQ(0, memcmp(buf.str().c_str(), copiedBuf.str().c_str(), copiedBuf.str().length()));
        std::istream is(&buf);
        {
            boost::archive::binary_iarchive ia(is, boost::archive::no_header);
            ia >> receivedEventA;
        }
        EXPECT_EQ(eventA, receivedEventA);
    }

    Pipe pipe;
    {
//        EventA *result = new EventA({});
        EventA result({1,2});

        std::stringbuf receivedBuf;
        receivedBuf.sputn(msg.data, msg.header.length);
        std::cout<<"len: "<<msg.header.length<<std::endl;
        std::istream is(&receivedBuf);
        {
            boost::archive::binary_iarchive ia(is, boost::archive::no_header);
            EXPECT_EQ(receivedBuf.str().length(), msg.header.length);
            EXPECT_EQ(0, memcmp(receivedBuf.str().c_str(), msg.data, msg.header.length));
            EXPECT_EQ('A', msg.getType());
//            if(msg.getType() == 'A')
//                ia >> result;
//            std::cout<<  result.intli[0]  <<std::endl;
        }
    }

//    pipe.writeOneMessage(msg);
//
//    Message *receivedMsg = pipe.readOneMessage();
//    EventA *receivedEvent = EventA::makeFromMsg(*receivedMsg);
//
//    EXPECT_EQ(eventA, *receivedEvent);
//
//    delete receivedEvent;
//    delete receivedMsg;
}

/* TODO: Message only have unique_ptr<stringbuf> (to avoid copy)
 * in this case, stringbuf already contains type, len (8byte) in front of byte array.
 * to do that, we must know the size of the raw data before put it to stringbuf.
 * caller of Message ctor should make this stringbuf by itself.
 * DO NOT USE memcpy.
 * DO NOT CONSIDER ENDIANESS in production code! (let libc++ handle endianess.)
 * */


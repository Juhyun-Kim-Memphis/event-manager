#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>

#include <gtest/gtest.h>
#include <ostream>
#include "../pipe.hpp"

TEST(BoostSerialization, testStringbufToByteArray) {
    std::stringbuf buf;
    buf.sputn("abc", 4);

    Pipe pipe;
    write(pipe.getWritefd(), buf.str().c_str(), buf.str().size());

    char charBuf[10];
    ssize_t nbyteRead = read(pipe.getReadfd(), charBuf, buf.str().size());

    std::stringbuf resBuf(charBuf);

    EXPECT_EQ(nbyteRead, buf.str().size());
    EXPECT_EQ(nbyteRead, 4);
    EXPECT_EQ(charBuf[2], buf.str().c_str()[2]);
    EXPECT_EQ(charBuf[3], buf.str().c_str()[3]);
    EXPECT_EQ(charBuf[3], '\0');
}

/////////////////////////////////////////////////////////////
// gps coordinate
// illustrates serialization for a simple type
class GPSPosition
{
public:
    GPSPosition(){};
    GPSPosition(int d, int m, float s) :
        degrees(d), minutes(m), seconds(s)
    {}

    bool operator==(const GPSPosition &rhs) const {
        return degrees == rhs.degrees &&
               minutes == rhs.minutes &&
               seconds == rhs.seconds;
    }

    bool operator!=(const GPSPosition &rhs) const {
        return !(rhs == *this);
    }

private:
    friend class boost::serialization::access;

    //Intrusive version of 'serialize'
    // When the class Archive corresponds to an output archive, the
    // & operator is defined similar to <<.  Likewise, when the class Archive
    // is a type of input archive the & operator is defined similar to >>.
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & degrees;
        ar & minutes;
        ar & seconds;
    }
    int degrees;
    int minutes;
    float seconds;
};

TEST(BoostSerialization, testTextArchive) {
    // create and open a character archive for output
    std::ofstream ofs("filename");
    const GPSPosition g(35, 59, 24.567f);

    // save data to archive
    {
        boost::archive::text_oarchive oa(ofs);
        // write class instance to archive
        oa << g;
    	// archive and stream closed when destructors are called
    }

    GPSPosition newg;
    {
        // create and open an archive for input
        std::ifstream ifs("filename");
        boost::archive::text_iarchive ia(ifs);
        // read class state from archive
        ia >> newg;
        // archive and stream closed when destructors are called
    }

    EXPECT_EQ(newg, g);
}

class EventForTest : public Event {
public:
    EventForTest() : Event('t'), data(4, 5, 4.3) {}

    EventForTest(char t) : Event(t), data(GPSPosition(3, 3, 1.1)) {}

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<Event>(*this);
        ar & data;
    }

    bool operator==(const EventForTest &rhs) const {
        return getEventID() == rhs.getEventID() &&
               data == rhs.data ;
    }

    bool operator!=(const EventForTest &rhs) const {
        return !(rhs == *this);
    }

    GPSPosition data;
};

TEST(BoostSerialization, testMessage) {
    EventForTest ev('d');

    std::stringbuf buf;
    std::ostream os(&buf);
    {
        boost::archive::binary_oarchive oa(os, boost::archive::no_header);
        oa << ev;
    }

    uint32_t length = buf.str().size();
    uint32_t msgSize = sizeof(uint32_t) + length;
    char *data = (char *)malloc(msgSize);
    memcpy(data , &length, sizeof(uint32_t));
    memcpy(data + sizeof(uint32_t), buf.str().c_str(), length); //TODO: make ev to be serialized first!

    Pipe pipe;
    write(pipe.getWritefd(), data, msgSize);

    ///////

    int len;
    char recvBuf[512];
    ssize_t nbyteRead;
    nbyteRead = read(pipe.getReadfd(), &len, sizeof(uint32_t)); //read length first
    EXPECT_EQ(sizeof(uint32_t), nbyteRead);
    EXPECT_EQ(length, len);

    nbyteRead = read(pipe.getReadfd(), recvBuf, len);
    EXPECT_EQ(len, nbyteRead);

    std::stringbuf resBuf;
    resBuf.sputn(recvBuf, nbyteRead);
    std::istream is(&resBuf);
    EventForTest resEv('l');
    {
        boost::archive::binary_iarchive ia(is, boost::archive::no_header);
        ia >> resEv;
    }

    EXPECT_EQ(ev.data, resEv.data);
    EXPECT_EQ(ev.getEventID(), resEv.getEventID());
    EXPECT_EQ(ev, resEv);
}

class Foo {
public:
    Foo(int in) {
        ptr = new int;
        *ptr = in;
        intli.push_back(in);
        intli.push_back(in+3);
    }

    virtual ~Foo() { delete ptr; }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & (*ptr);
        ar & intli;
    }

    bool operator==(const Foo &rhs) const {
        return *ptr == *rhs.ptr;
    }

    int *ptr;
    std::vector<int> intli;
};

TEST(BoostSerialization, testSerializationWithPointerMember) {
    Foo foo(3);
    std::stringbuf buf;
    std::ostream os(&buf);
    {
        boost::archive::binary_oarchive oa(os, boost::archive::no_header);
        oa << foo;
    }

    ///////////////// buf I/O

    std::istream is(&buf);
    Foo result(1);
    result.intli.pop_back();
    int *ptrValOfMember = result.ptr;
    {
        boost::archive::binary_iarchive ia(is, boost::archive::no_header);
        ia >> result;
    }
    EXPECT_EQ(foo, result);
    EXPECT_NE(foo.ptr, result.ptr);
    EXPECT_EQ(foo.intli, result.intli);
}

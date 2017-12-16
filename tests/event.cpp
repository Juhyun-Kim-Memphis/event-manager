#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <gtest/gtest.h>
#include "../Pipe.hpp"

/////////////////////////////////////////////////////////////
// gps coordinate
//
// illustrates serialization for a simple type
//
class gps_position
{
public:
    gps_position(){};
    gps_position(int d, int m, float s) :
        degrees(d), minutes(m), seconds(s)
    {}

    bool operator==(const gps_position &rhs) const {
        return degrees == rhs.degrees &&
               minutes == rhs.minutes &&
               seconds == rhs.seconds;
    }

    bool operator!=(const gps_position &rhs) const {
        return !(rhs == *this);
    }

private:
    friend class boost::serialization::access;
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

TEST(BoostSerialization, testTextArchive) {
    // create and open a character archive for output
    std::ofstream ofs("filename");
    const gps_position g(35, 59, 24.567f);

    // save data to archive
    {
        boost::archive::text_oarchive oa(ofs);
        // write class instance to archive
        oa << g;
    	// archive and stream closed when destructors are called
    }

    gps_position newg;
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

TEST(BoostSerialization, testBinaryArchive) {
    const gps_position g(35, 59, 24.567f);
    std::stringbuf buf;
    std::ostream os(&buf);

    {
        boost::archive::binary_oarchive oa(os, boost::archive::no_header);
        oa << g;
        std::cout<< " ["<< buf.str()<<"]\n";
    }

    Pipe pipe;
    write(pipe.getWritefd(), buf.str().c_str(), buf.str().size());


    ////////////////////////

    char charBuf[512];
    ssize_t nbyteRead = read(pipe.getReadfd(), charBuf, buf.str().size());
    std::stringbuf resBuf;
    resBuf.sputn(charBuf, nbyteRead);
//    std::ostream os2(&buf);
//    os2 << charBuf;
//    std::cout<< " [" << resBuf.str()<<"]\n";
//    std::cout<< " [" << nbyteRead
//                     <<", "<< buf.str().size()+1
//                     <<", "<< resBuf.str().size()+1 <<"]\n";

    std::istream is(&resBuf);
    gps_position newg;
    {
        boost::archive::binary_iarchive ia(is, boost::archive::no_header);
        // read class state from archive
        ia >> newg;
        // archive and stream closed when destructors are called
    }

    EXPECT_EQ(newg, g);
}



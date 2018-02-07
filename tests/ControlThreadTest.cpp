#include <thread>
#include <Pipe.hpp>
#include "gtest/gtest.h"
#include "boost/asio.hpp"

using namespace boost;

class DummyClient {
public:
    void connect(const char *ipAddr, int port){
        asio::ip::tcp::endpoint ep(asio::ip::address::from_string("127.0.0.1"), port);
        asio::io_service ios;
        asio::ip::tcp::socket sock(ios, ep.protocol());
        sock.connect(ep);
        std::cout<<"Client: I'm connected!\n";
        std::string buf = "Hello, world.";
        sock.send(asio::buffer(buf));
    }
};

TEST(ControlThread, testAccept) {
    unsigned short portNumOfServer = 3333;

    asio::ip::tcp::endpoint epServer(asio::ip::address_v4::any(), portNumOfServer);
    asio::io_service ios;

    asio::ip::tcp::acceptor acceptor(ios, epServer.protocol());

    boost::system::error_code ec;

    acceptor.bind(epServer, ec);
    acceptor.listen();

    asio::ip::tcp::socket sock(ios);

    std::thread client([](){
        DummyClient client;
        client.connect("127.0.0.1", 3333);
    });




    ///////////
    acceptor.async_accept(sock, [&sock](const boost::system::error_code& error){
        if (!error){
            const int receiveBufSize = 1024;
            char receiveBuf[receiveBufSize];

            std::cout << "async accept success!" << std::endl;
            sock.async_receive(asio::buffer(receiveBuf), [&receiveBuf](const boost::system::error_code& error, std::size_t byteTransferred){
                std::cout << "Message arrived from the connected client: " << receiveBuf << std::endl;
            });
        }
    });

    ios.run();
    /* TODO: move async_receive here. why not working? */
    /////////////////////////


    client.join();
//    EXPECT_EQ(ec, 0);
//    EXPECT_EQ(connectedSocketPort, ephemeralPort);
}

TEST(ControlThread, testPipeAsAsioStreamDescriptor) {
    Pipe pipe;
    asio::io_service ios;

    asio::posix::stream_descriptor readEnd(ios, pipe.reader().getFD());
    asio::posix::stream_descriptor writeEnd(ios, pipe.writer().getFD());

    //// write ////
    std::string buf = "Hello, world.";
//    writeEnd.write_some(asio::buffer(buf));
    writeEnd.async_write_some(asio::buffer(buf), [](const boost::system::error_code& error, std::size_t byteTransferred){
        std::cout<<"write success!!!! "<<byteTransferred<<std::endl;
    });


    //// read ////
    const int receiveBufSize = 1024;
    char receiveBuf[receiveBufSize];
//    readEnd.read_some(asio::buffer(receiveBuf));
    readEnd.async_read_some(asio::buffer(receiveBuf), [&receiveBuf](const boost::system::error_code& error, std::size_t byteTransferred){
        std::cout<<receiveBuf<<", byteTransferred: "<<byteTransferred<<std::endl;
    });

    ios.run();

}

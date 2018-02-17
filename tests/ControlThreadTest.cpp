#include <thread>
#include <Pipe.hpp>
#include <ControlThread.hpp>
#include "gtest/gtest.h"
#include "boost/asio.hpp"

using namespace boost;

class DummyClient {
public:
    void connect(const char *ipAddr, int port){
        asio::ip::tcp::endpoint ep(asio::ip::address::from_string("127.0.0.1"), port);
        asio::io_service ios;
        sock.reset(new asio::ip::tcp::socket(ios, ep.protocol()));
        sock->connect(ep);
    }

    void sendString(std::string message) {
        sock->send(asio::buffer(message));
    }

    void closeConnection() {
        sock->close();
    }
private:
    std::unique_ptr<asio::ip::tcp::socket> sock;
};

/* this test use tcp socket. */
//TEST(ControlThread, testAccept) {
//    unsigned short portNumOfServer = 33335;
//    asio::ip::tcp::endpoint epServer(asio::ip::address_v4::any(), portNumOfServer);
//    asio::io_service ios;
//    asio::ip::tcp::acceptor acceptor(ios, epServer.protocol());
//    boost::system::error_code ec;
//    acceptor.bind(epServer, ec);
//    acceptor.listen();
//    asio::ip::tcp::socket sock(ios);
//    std::stringstream ss;
//
//    std::thread client([](){
//        DummyClient client;
//        client.connect("127.0.0.1", 33335);
//        client.sendString(std::string("Hello, world."));
//        client.closeConnection();
//    });
//
//    acceptor.async_accept(sock, [&sock, &ss](const boost::system::error_code& error){
//        if (!error){
//            const int receiveBufSize = 1024;
//            char receiveBuf[receiveBufSize];
//
//            sock.async_receive(asio::buffer(receiveBuf), [&receiveBuf, &ss](const boost::system::error_code& error, std::size_t byteTransferred){
//                ss << "Message arrived from the connected client: " << receiveBuf << std::endl;
//            });
//        }
//    });
//
//    ios.run();
//    /* TODO: move async_receive here. why not working? */
//
//    EXPECT_EQ(std::string("Message arrived from the connected client: Hello, world.\n"), ss.str());
//
//    ios.run();
//    sock.close();
//    acceptor.close();
//    client.join();
//}

TEST(ControlThread, testPipeAsAsioStreamDescriptor) {
    Pipe pipe;
    asio::io_service ios;

    asio::posix::stream_descriptor readEnd(ios, pipe.reader().getFD());
    asio::posix::stream_descriptor writeEnd(ios, pipe.writer().getFD());

    //// write ////
    std::string buf = "Hello, world.";
    // writeEnd.write_some(asio::buffer(buf));
    writeEnd.async_write_some(asio::buffer(buf), [](const boost::system::error_code& error, std::size_t byteTransferred){
        std::cout<<"write success!!!! "<<byteTransferred<<std::endl;
    });

    //// read ////
    const int receiveBufSize = 1024;
    char receiveBuf[receiveBufSize];
    // readEnd.read_some(asio::buffer(receiveBuf));
    readEnd.async_read_some(asio::buffer(receiveBuf), [&receiveBuf](const boost::system::error_code& error, std::size_t byteTransferred){
        std::cout<<receiveBuf<<", byteTransferred: "<<byteTransferred<<std::endl;
    });

    ios.run();
}

/* this test use tcp socket. */
//TEST(ControlThread, testMultipleClient) {
//    ControlThread cthr;
//    std::thread controlThread(std::bind(&ControlThread::start, &cthr, 33336));
//
//    auto clientRoutine = [](){
//        DummyClient client;
//        client.connect("127.0.0.1", 33336);
//    };
//
//    std::vector< std::unique_ptr<std::thread> > clients;
//    for(int i = 0; i < 3; i++){
//        std::unique_ptr<std::thread> th(new std::thread(clientRoutine));
//        clients.push_back(std::move(th));
//    }
//
//    std::this_thread::sleep_for(std::chrono::seconds(1));
//    cthr.stop();
//
//    controlThread.join();
//    for (auto& th : clients)
//        th->join();
//}



TEST(ControlThread, testReadPipeOfControlThread) {
    ControlThread cthr;

    //// write ////
    std::string msg("Hello, You.\n");
    std::shared_ptr<Message> toBeSent(RAIIWrapperMessage::newMessageByAllocatingAndCopying(66, msg.c_str(), msg.length()));
    cthr.getPipeWriter().writeOneMessage(*toBeSent.get());

    /* send Stop msg to CTHR beforehand */
    asio::posix::stream_descriptor toCthr(cthr.getIOService(), cthr.getPipeWriter().getFD());
    Message stop = Message::makeDummyMessage(ControlThread::STOP);
    asio::write(toCthr, asio::buffer(stop.getHeader(), sizeof(Message::Header)));

    //// read ////
    cthr.runUntilGettingStop();

    std::cout<<"Reached here .\n";
}



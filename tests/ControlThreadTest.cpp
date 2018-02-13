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

TEST(ControlThread, testAccept) {
    unsigned short portNumOfServer = 33335;
    asio::ip::tcp::endpoint epServer(asio::ip::address_v4::any(), portNumOfServer);
    asio::io_service ios;
    asio::ip::tcp::acceptor acceptor(ios, epServer.protocol());
    boost::system::error_code ec;
    acceptor.bind(epServer, ec);
    acceptor.listen();
    asio::ip::tcp::socket sock(ios);
    std::stringstream ss;

    std::thread client([](){
        DummyClient client;
        client.connect("127.0.0.1", 33335);
        client.sendString(std::string("Hello, world."));
        client.closeConnection();
    });

    acceptor.async_accept(sock, [&sock, &ss](const boost::system::error_code& error){
        if (!error){
            const int receiveBufSize = 1024;
            char receiveBuf[receiveBufSize];

            sock.async_receive(asio::buffer(receiveBuf), [&receiveBuf, &ss](const boost::system::error_code& error, std::size_t byteTransferred){
                ss << "Message arrived from the connected client: " << receiveBuf << std::endl;
            });
        }
    });

    ios.run();
    /* TODO: move async_receive here. why not working? */

    EXPECT_EQ(std::string("Message arrived from the connected client: Hello, world.\n"), ss.str());

    ios.run();
    sock.close();
    acceptor.close();
    client.join();
}

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

TEST(ControlThread, testMultipleClient) {
    ControlThread cthr;
    std::thread controlThread(std::bind(&ControlThread::start, &cthr, 33336));

    auto clientRoutine = [](){
        DummyClient client;
        client.connect("127.0.0.1", 33336);
    };

    std::vector< std::unique_ptr<std::thread> > clients;
    for(int i = 0; i < 3; i++){
        std::unique_ptr<std::thread> th(new std::thread(clientRoutine));
        clients.push_back(std::move(th));
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    cthr.stop();

    controlThread.join();
    for (auto& th : clients)
        th->join();
}

struct PipeInputStream {
    PipeInputStream(asio::io_service &ios, int readFD) :
            totalBytesRead(0),
            readPipe(new asio::posix::stream_descriptor(ios, readFD)),
            ioService(ios) {}

    std::shared_ptr<asio::posix::stream_descriptor> readPipe;
    asio::io_service &ioService;

    asio::streambuf buf;
    std::size_t totalBytesRead;

    Message::Header tempHeader;
};

void onReadMessageHeader(const boost::system::error_code& ec, std::size_t bytesRead, std::shared_ptr<PipeInputStream> pis);

void onReadPayload(const boost::system::error_code& ec, std::size_t bytesRead, std::shared_ptr<PipeInputStream> pis) {
    if (ec != 0) {
        std::cerr<<"Pipe API Error:"<<ec.message()<<"("<<ec.value()<<")"<<std::endl;
        return;
    }

    pis->totalBytesRead += bytesRead;

    pis->buf.commit(bytesRead);
    std::istream is(&pis->buf);
    std::string contents;
    std::getline(is, contents);


    std::cout<<"received:["<< contents<<"] size: "<< contents.size()<<", "
             <<"byteRead: "<<bytesRead<<", "
             <<"input size: "<<pis->buf.size()<<", "
             <<"temp header: "<<pis->tempHeader.type<<", "<<pis->tempHeader.length<<"\n\n";

    pis->tempHeader.reset();
    asio::async_read(*pis->readPipe.get(), asio::buffer(&pis->tempHeader, sizeof(Message::Header)),
                     std::bind(onReadMessageHeader, std::placeholders::_1, std::placeholders::_2, pis));
}

void onReadMessageHeader(const boost::system::error_code& ec, std::size_t bytesRead, std::shared_ptr<PipeInputStream> pis) {
    if (ec != 0) {
        std::cerr<<"Pipe API Error:"<<ec.message()<<"("<<ec.value()<<")"<<std::endl;
        return;
    }

    pis->totalBytesRead += bytesRead;

    std::cout<<"onReadMessageHeader: byteRead: "<<bytesRead<<", "
             <<"input size: "<<pis->buf.size()<<", "
             <<"temp header: "<<pis->tempHeader.type<<", "<<pis->tempHeader.length<<"\n";

    if(pis->tempHeader.type == ControlThread::STOP){
        /* TODO: remove or change this as log*/
        std::cout<<"In onReadMessageHeader: received STOP message. \n";
        pis->ioService.stop();
    }

    /* TODO: to insert switch statement here. */

    auto payloadSize = pis->tempHeader.length;

    if( payloadSize == 0 )
        asio::async_read(*pis->readPipe.get(), asio::buffer(&pis->tempHeader, sizeof(Message::Header)),
                     std::bind(onReadMessageHeader,
                               std::placeholders::_1,
                               std::placeholders::_2, pis));
    else if( payloadSize > 0 ){
        asio::async_read(*pis->readPipe.get(), pis->buf, asio::transfer_exactly(payloadSize),
                         std::bind(onReadPayload,
                                   std::placeholders::_1,
                                   std::placeholders::_2, pis));
    }
    else {
        std::cerr<<"Pipe Message Error: Invalid Payload Size: ["<<payloadSize<<"]"<<std::endl;
        return;
    }

}

TEST(ControlThread, testReadPipeOfControlThread) {
    Pipe pipe;
    asio::io_service ios;

    //// write ////
    asio::posix::stream_descriptor writeEnd(ios, pipe.writer().getFD());

    std::string msg("Hello, You.\n");
    std::shared_ptr<Message> toBeSent(RAIIWrapperMessage::newMessageByAllocatingAndCopying(66, msg.c_str(), msg.length()));
    pipe.writer().writeOneMessage(*toBeSent.get());

    Message stop = Message::makeDummyMessage(ControlThread::STOP);
    writeEnd.write_some(asio::buffer(stop.getHeader(), sizeof(Message::Header)));

    //// read ////
    std::shared_ptr<PipeInputStream> pis(new PipeInputStream(ios, pipe.reader().getFD()));

    asio::async_read(*pis->readPipe.get(), asio::buffer(&pis->tempHeader, sizeof(Message::Header)),
                     std::bind(onReadMessageHeader, std::placeholders::_1, std::placeholders::_2, pis));

    ios.run();

    std::cout<<"Reached here .\n";
}

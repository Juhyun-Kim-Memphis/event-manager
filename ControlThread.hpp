#ifndef EVENT_MANAGER_CONTROLTHREAD_HPP
#define EVENT_MANAGER_CONTROLTHREAD_HPP

#include <vector>
#include <boost/asio.hpp>
#include <iostream>
#include "Pipe.hpp"

using namespace boost;

class ListeningSocket {
public:
    ListeningSocket(asio::io_service &ioService, unsigned short listeningPortNum)
            : isStopped(false), ioService(ioService),
              acceptor(ioService, asio::ip::tcp::endpoint(asio::ip::address_v4::any(), listeningPortNum)) {}

    void start() {
        acceptor.listen();
        acyncAccept();
    }

    void stop() {
        isStopped.store(true);
    }

private:
    using Socket = asio::ip::tcp::socket;

    void registerClient(const std::shared_ptr<Socket> &newClient) {
        connectedClients.push_back(newClient);
    }

    void acyncAccept() {
        std::shared_ptr<Socket> clientSocket(new asio::ip::tcp::socket(ioService));
        acceptor.async_accept(*clientSocket.get(), [this, clientSocket](const boost::system::error_code& error)
        {
            if(error != 0) /* TODO: use throw */
                std::cerr<<"Socket API Error:"<<error.message()<<"("<<error.value()<<")"<<std::endl;
            else {
                registerClient(clientSocket);

                if(isStopped.load())
                    acceptor.close();
                else
                    acyncAccept();
            }
        });
    }

    std::atomic<bool> isStopped;
    asio::io_service& ioService;
    asio::ip::tcp::acceptor acceptor;

public:
    /*TODO: move to ControlThread */
    std::vector<std::shared_ptr<Socket>> connectedClients;
};

class ControlThread {
public:
    static constexpr Message::TypeID STOP = 7453584;

    void start(unsigned short port){
        listener.reset(new ListeningSocket(ioService, port));
        listener->start();

        /* block here until getting STOP msg */
        ioService.run();
    }

    /* may be called by other thread */
    void stop(){
        listener->stop();
        ioService.stop();
    }

    PipeWriter &getPipeWriter(){ /* TODO: remove */
        return pipe.writer();
    }

private:
    std::unique_ptr<ListeningSocket> listener;
    Pipe pipe;

    asio::io_service ioService;

    /*asio::streambuf pipeReadBuf;*/
    std::unique_ptr<char[]> pipeReadBuf;
    std::size_t totalBytesRead;
    unsigned int bufSize;
};


#endif //EVENT_MANAGER_CONTROLTHREAD_HPP

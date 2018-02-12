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
    void start(unsigned short port){
        listener.reset(new ListeningSocket(ioService, port));

        registerMyPipe();
        listener->start();
        ioService.run();

        /*TODO: remove*/
        std::cout<<"clientList:\n";
        for(auto &e : listener->connectedClients) {
            std::cout<<"client: "<<e->remote_endpoint()<<"\n";
        }
    }

    /* may be called by other thread */
    void stop(){
        listener->stop();
        ioService.stop();
    }

    PipeWriter &getPipeWriter(){ /* TODO: remove */
        return pipe.writer();
    }

    /* TODO: change to private */
    void registerMyPipe() {
        bufSize = 1024;
        totalBytesRead = 0;
        pipeReadBuf.reset(new char[bufSize]);

        asio::posix::stream_descriptor readEnd(ioService, pipe.reader().getFD());
        readEnd.async_read_some(asio::buffer(pipeReadBuf.get(), bufSize), [this, &readEnd]
                (const boost::system::error_code& error, std::size_t byteTransferred){
            if( error != 0 ){
                std::cerr<<"Pipe API Error:"<<error.message()<<"("<<error.value()<<")"<<std::endl;
            }

            totalBytesRead += byteTransferred;

            if (totalBytesRead == byteTransferred)
                return;

            std::string contents(pipeReadBuf.get());
            std::cout<< contents <<", byteTransferred: "<<byteTransferred<<std::endl;
        });
    }

    asio::io_service ioService;

private:
    std::unique_ptr<ListeningSocket> listener;
    Pipe pipe;

    /*asio::streambuf pipeReadBuf;*/
    std::unique_ptr<char[]> pipeReadBuf;
    std::size_t totalBytesRead;
    unsigned int bufSize;
};


#endif //EVENT_MANAGER_CONTROLTHREAD_HPP

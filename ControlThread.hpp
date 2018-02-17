#ifndef EVENT_MANAGER_CONTROLTHREAD_HPP
#define EVENT_MANAGER_CONTROLTHREAD_HPP

#include <vector>
#include <boost/asio.hpp>
#include <iostream>
#include "Pipe.hpp"
#include "ConnectAcceptor.hpp"

using namespace boost;

struct PipeInputStream;

class ControlThread {
public:
    static constexpr Message::TypeID STOP = 7453584;

    ControlThread();

    void runUntilGettingStop();

    /* may be called by other threads. */
    /* TODO: remove */
    PipeWriter &getPipeWriter() {
        return pipe.writer();
    }

    asio::io_service &getIOService() {
        return ioService;
    }

private:
    asio::io_service ioService;

    Pipe pipe;
    std::shared_ptr<PipeInputStream> pis;
};


struct PipeInputStream {
    PipeInputStream(asio::io_service &ios, int readFD) :
            totalBytesRead(0),
            readPipe(new asio::posix::stream_descriptor(ios, readFD)),
            ioService(ios) {}

    std::shared_ptr<asio::posix::stream_descriptor> readPipe;
    asio::io_service &ioService;

    /* pipe read buffer for Message Header. */
    Message::Header tempHeader;

    /* pipe read buffer for Message payload. */
    asio::streambuf buf;
    std::size_t totalBytesRead;
};



#endif //EVENT_MANAGER_CONTROLTHREAD_HPP

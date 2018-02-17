//
// Created by juhyun on 18. 2. 7.
//

#include "ControlThread.hpp"

ControlThread::ControlThread() : pis(new PipeInputStream(ioService, pipe.reader().getFD())) {}

void onReadMessageHeader(const boost::system::error_code& ec, std::size_t bytesRead, std::shared_ptr<PipeInputStream> pis);

void ControlThread::runUntilGettingStop() {

    /* initialize Acceptor Socket and start asynchronous accept. */
    // std::unique_ptr<ListeningSocket> listener;
    // listener.reset(new ListeningSocket(ioService, port));
    // listener->start();

    asio::async_read(*pis->readPipe.get(), asio::buffer(&pis->tempHeader, sizeof(Message::Header)),
                     std::bind(onReadMessageHeader, std::placeholders::_1, std::placeholders::_2, pis));

    /* block here until getting STOP msg */
    ioService.run();
}

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
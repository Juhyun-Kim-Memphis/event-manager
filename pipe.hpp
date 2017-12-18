#ifndef EVENT_MANAGER_PIPE_HPP
#define EVENT_MANAGER_PIPE_HPP
#ifdef _WIN32
#include <mingw.thread.h>
#include <fcntl.h>
#define pipe(fds) _pipe(fds,4096, _O_BINARY)
#endif

#include <unistd.h>
#include <string>
#include <iostream>
#include "Event.hpp"



struct Header {
    Header() {}
    Header(int type, size_t length) : type(type), length(length) {}
    int type;
    size_t length;
};//TODO: move inside to Pipe or Message.

/* can be event or task */
class Message {
public:
    Message() : type(0), length(0), data(nullptr) {}
    Message(int type, size_t length, char *data) : type(type), length(length), data(data) {}
    ~Message() { delete data; }

    Header getHeader() const { return Header(type, length); }
    char *getData() const { return data; }

    bool operator==(const Message &rhs) const {
        return type == rhs.type &&
               length == rhs.length &&
               memcmp(data, rhs.data, length) == 0;
        /* WARNING: memcmp can cause SEGV */
    }

    bool operator!=(const Message &rhs) const {
        return !(rhs == *this);
    }
private:
    int type; /* TODO: enum?*/
    size_t length; /* TODO: use c++ streambuf ? */
    char *data;
};

class Pipe {
public:
    Pipe() {
        if (pipe(fd) == -1)
            throw std::string("pipe creation fails.");
    }

    virtual ~Pipe() {
        close(getReadfd());
        close(getWritefd());
    }

    int getReadfd() { //TODO: remove
        return fd[0];
    }

    int getWritefd() {
        return fd[1];
    }

    /* TODO: how to use just read, write as name of method? */
    void writeOneMessage(const Message &msg) {
        Header h = msg.getHeader();
        char *sndBuf = new char[sizeof(Header) + h.length];
        memcpy(sndBuf, &h, sizeof(Header));
        memcpy(sndBuf + sizeof(Header), msg.getData(), h.length);
        write(getWritefd(), sndBuf, sizeof(Header) + h.length);
        delete sndBuf;
    }

    Message *readOneMessage(){
        Header header;
        read(getReadfd(), &header, sizeof(Header));
        char *buf = (char *)malloc(header.length); //TODO: use streambuf or new
        read(getReadfd(), buf, header.length); //TODO: add Assertion.
        return new Message(header.type, header.length, buf);
    }

private:
    int fd[2];
};

class PipeInputGate {
public:
    explicit PipeInputGate(int writeFd) : writeFd(writeFd) {}
    explicit PipeInputGate(Pipe pipe) : writeFd(pipe.getWritefd()) {}

    void sendEvent(Event &input) {
        char released = 'r';
        write(writeFd, &released, 1);
    }

private:
    int writeFd;
};

#endif //EVENT_MANAGER_PIPE_HPP

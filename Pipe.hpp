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

/* can be event or task */
class Message {
public:
    struct Header {
        Header() {}
        Header(uint32_t type, uint32_t length) : type(type), length(length) {}
        uint32_t type; /* TODO: enum?*/
        uint32_t length;
    };

    Message(uint32_t type, uint32_t length, const char *src) : header(type, length), data(new char[length]) {
        memcpy(data, src, length); /* TODO: too many copy operation of raw data. reduce them.*/
    }

    ~Message() {
        delete data;
    }

    int getType(){ return header.type; }

    char *makeSerialzedMessage() const {
        char *buf = new char[sizeof(Header) + header.length];
        memcpy(buf, &header, sizeof(Header));
        memcpy(buf + sizeof(Header), data, header.length);
        return buf;
    }
    size_t getSerialzedMessageSize() const {
        return sizeof(Header) + header.length;
    }

    bool operator==(const Message &rhs) const {
        return header.type == rhs.header.type &&
               header.length == rhs.header.length &&
               memcmp(data, rhs.data, header.length) == 0;
        /* WARNING: memcmp can cause SEGV */
    }

    bool operator!=(const Message &rhs) const {
        return !(rhs == *this);
    }

    friend std::ostream &operator<<(std::ostream &os, const Message &message) {
        os << "type: " << message.header.type << " length: " << message.header.length << " data: " << message.data;
        return os;
    }

public:
    Header header;
    char *data; /* TODO: "onwership?", "dtor delete?", "unique_ptr?" */
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
        char *sendBuffer = msg.makeSerialzedMessage();
        write(getWritefd(), sendBuffer, msg.getSerialzedMessageSize());
        delete sendBuffer;
    }

    Message *readOneMessage(){
        Message::Header header;
        read(getReadfd(), &header, sizeof(Message::Header));
        char *buf = (char *)malloc(header.length); //TODO: use streambuf or new
        read(getReadfd(), buf, header.length); //TODO: add Assertion.
        return new Message(header.type, header.length, buf);
        /* TODO: use smart pointer */
    }

private:
    int fd[2];
};

class PipeWriter {

private:
    int writeFd;
};

#endif //EVENT_MANAGER_PIPE_HPP

#ifndef EVENT_MANAGER_PIPE_HPP
#define EVENT_MANAGER_PIPE_HPP
#ifdef _WIN32
#include <mingw.thread.h>
#include <fcntl.h>
#endif

#include <unistd.h>
#include <string>
#include <iostream>
#include "Event.hpp"

/* can be event or task */
class Message {
public:
    using ID = uint32_t;

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
        delete[] data;
    }

    int getType(){ return header.type; }

    char *makeSerializedMessage() const {
        char *buf = new char[sizeof(Header) + header.length];
        memcpy(buf, &header, sizeof(Header));
        memcpy(buf + sizeof(Header), data, header.length);
        return buf;
    }
    size_t getSerializedMessageSize() const {
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

    static Message makeDummyMessage(ID id = 0) {
        return Message(id);
    }

public:
    Header header;
    char *data; /* TODO: "onwership?", "dtor delete?", "unique_ptr?" */

    /* header only message */
    Message(uint32_t type) : header(type, 0), data(nullptr) {}
};

class PipeWriter {
public:
    PipeWriter() {}

    void setFd(int fd) { writeFd = fd; } /* TODO: remove this. */

    void writeOneMessage(const Message &msg) const {
        char *sendBuffer = msg.makeSerializedMessage();
        write(writeFd, sendBuffer, msg.getSerializedMessageSize());
        delete sendBuffer;
    }

    /*TODO: remove */
    ssize_t writeBytes(const void *data, size_t len) const{
        return write(writeFd, data, len);
    }

    int getWriteFd() const { return writeFd; }

private:
    PipeWriter(const PipeWriter& pw) = delete;
    PipeWriter(PipeWriter&& pw) = delete;
    int writeFd;
};

class PipeReader {
public:
    PipeReader() {}
    void setFd(int fd) { readFd = fd; }

    Message *readOneMessage() const {
        Message::Header header;
        read(readFd, &header, sizeof(Message::Header));
        char *buf = (char *)malloc(header.length); //TODO: use streambuf or new.
        read(readFd, buf, header.length); //TODO: add Assertion.
        return new Message(header.type, header.length, buf);
        /* TODO: use smart pointer, or delete buf */
    }

    ssize_t readBytes(void *buf, size_t len) const {
        return read(readFd, buf, len);
    }

private:
    PipeReader(const PipeReader& pw) = delete;
    PipeReader(PipeReader&& pw) = delete;
    int readFd;
};

class Pipe {
public:
    Pipe() {
        if (pipeSyscall(fd) == -1)
            throw std::string("pipe creation fails.");
        readEnd.setFd(fd[0]);
        writeEnd.setFd(fd[1]);
    }

    virtual ~Pipe() {
        close(fd[0]);
        close(fd[1]);
    }

    PipeReader& reader(){ return readEnd; }
    PipeWriter& writer(){ return writeEnd; }

private:
    int fd[2];
    PipeReader readEnd;
    PipeWriter writeEnd;

    int pipeSyscall(int *fds) {
#ifdef _WIN32
        return _pipe(fds,4096, _O_BINARY);
#else
        return pipe(fds);
#endif
    }
};



#endif //EVENT_MANAGER_PIPE_HPP

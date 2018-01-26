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
#include "Message.hpp"

class PipeWriter {
public:
    PipeWriter() {}

    void setFd(int fd) { writeFd = fd; } /* TODO: remove this. */

    void writeOneMessage(const Message &msg) const {
        char *sendBuffer = msg.makeSerializedMessage();
        write(writeFd, sendBuffer, msg.getSerializedMessageSize());
        delete[] sendBuffer;
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
        std::shared_ptr<char> bufWrapper(new char[header.length], std::default_delete<char[]>());
        read(readFd, bufWrapper.get(), header.length); //TODO: add Assertion.
        return RAIIWrapperMessage::newMessageByOutsidePayload(header.type, bufWrapper, header.length);
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

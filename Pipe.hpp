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


class Pipe {
public:
    Pipe() {
        if (pipe(fd) == -1)
            throw std::string("pipe creation fails.");
    }

    int getReadfd() {
        return fd[0];
    }

    int getWritefd() {
        return fd[1];
    }

private:
    int fd[2];
};

#endif //EVENT_MANAGER_PIPE_HPP

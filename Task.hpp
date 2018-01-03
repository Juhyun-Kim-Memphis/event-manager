#ifndef EVENT_MANAGER_TASK_HPP
#define EVENT_MANAGER_TASK_HPP

#include <unistd.h>
#include <map>
#include "Pipe.hpp"
#include "Event.hpp"


using namespace std;

class Task {
public:
    class UnimplementedHandle : public std::exception {
    public:
        const char* what() const noexcept {return "UnimplementedHandle!\n";}
    };

    Task() :  state(INITIAL) {}

    virtual void start() = 0;
    virtual void handle(Message *msg) {
        throw UnimplementedHandle();
    };
    bool hasQuit();

    /* TODO: make public method to force to quit the task */

protected:
    void quit();

    enum State {
        INITIAL,
        TERMINATED
    };
    State state; //TODO: make TaskState class
};

#endif //EVENT_MANAGER_TASK_HPP

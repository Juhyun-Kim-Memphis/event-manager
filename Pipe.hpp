//
// Created by user on 2017-10-21.
//

#ifndef EVENT_MANAGER_PIPE_HPP
#define EVENT_MANAGER_PIPE_HPP


class PipeContainer {
public:
    PipeContainer(int wp1, int wp2) : writePipe1(wp1), writePipe2(wp2) {}
    int writePipe1;
    int writePipe2;
};

extern PipeContainer *globalPipes;


#endif //EVENT_MANAGER_PIPE_HPP

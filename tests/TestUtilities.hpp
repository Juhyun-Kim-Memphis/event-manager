//
// Created by juhyun on 17. 11. 3.
//

#ifndef EVENT_MANAGER_TESTUTILITIES_HPP
#define EVENT_MANAGER_TESTUTILITIES_HPP

namespace TestUtil {
    void sendTriggeringEvent(Task *task){
        char moveToNext = 'n';
        write(task->getWritePipeFd(), &moveToNext, 1);
    }
}

#endif //EVENT_MANAGER_TESTUTILITIES_HPP

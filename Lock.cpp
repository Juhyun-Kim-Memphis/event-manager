#include <unistd.h>
#include <iostream>
#include "Lock.hpp"

bool LockUsingAOP::acquire(int writePipeFdOfRequester) {
    if (aop_cas(&lockVal, 1, 0) == 0) {
        owner = writePipeFdOfRequester;
        return true;
    } else {
        waiters.push_back(writePipeFdOfRequester);
        return false;
    }
}

void LockUsingAOP::release() {
    /* TODO: use aop */
    Lock::release();
}

long LockUsingAOP::aop_cas(volatile long *vp, long nv, long ov) {
#ifndef _WIN32
    asm __volatile__(
    "lock ; " "cmpxchgq %1,(%3)"
    : "=a" (nv)
    : "r" (nv), "a" (ov), "r" (vp)
    : "cc", "memory"
    );
#endif
    return nv;
}

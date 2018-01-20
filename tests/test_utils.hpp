#ifndef EVENT_MANAGER_TEST_UTILS_HPP
#define EVENT_MANAGER_TEST_UTILS_HPP

#include <ctime>

/* Wait for a certain condition to pass (with timeout)
 * 'actual' should be evaluated lazily.
 * */
template<typename T>
bool WAIT_FOR_EQ(T expected, const std::function<T()> &actual, int millisecondsToTimeout = 500){
    clock_t startTime = clock();
    constexpr int CLOCKS_PER_MILLISEC = CLOCKS_PER_SEC / 1000;
    while(expected != actual()){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        std::atomic_thread_fence(std::memory_order_seq_cst);
        int millisecsPassed = (clock() - startTime) / CLOCKS_PER_MILLISEC;
        if( millisecsPassed >= millisecondsToTimeout)
            return false; /* timeout */
    }
    return true;
}

#endif //EVENT_MANAGER_TEST_UTILS_HPP

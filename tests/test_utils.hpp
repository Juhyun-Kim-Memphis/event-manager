#ifndef EVENT_MANAGER_TEST_UTILS_HPP
#define EVENT_MANAGER_TEST_UTILS_HPP

#include <ctime>

/* Wait for a certain condition to pass (with timeout)
 * 'actual' should be evaluated lazily.
 * But, I'm satisfied with l-value referenve for now
 * (Consider: class template using std::function<>)
 * */
template <typename T, typename W>
bool WAIT_FOR_EQ(T expected, W& actual, int millisecondsToTimeout = 500){
    clock_t startTime = clock();
    constexpr int CLOCKS_PER_MILLISEC = CLOCKS_PER_SEC / 1000;

    while(expected != actual){
        int millisecsPassed = (clock() - startTime) / CLOCKS_PER_MILLISEC;
        if( millisecsPassed >= millisecondsToTimeout)
            return false; /* timeout */
    }
    return true;
}

#endif //EVENT_MANAGER_TEST_UTILS_HPP

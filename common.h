#ifndef __COMMON_H
#define __COMMON_H

#include <chrono>

inline auto time_diff_us(std::chrono::steady_clock::time_point t1,
                                       std::chrono::steady_clock::time_point t2)
{
    return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

inline auto get_timestamp() {
    return std::chrono::steady_clock::now();
}

#endif // __COMMON_H

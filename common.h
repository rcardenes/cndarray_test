#ifndef __COMMON_H
#define __COMMON_H

#include <chrono>
#include <ios>
#include <iostream>

inline auto time_diff_us(std::chrono::steady_clock::time_point t1,
                                       std::chrono::steady_clock::time_point t2)
{
    return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

inline auto get_timestamp() {
    return std::chrono::steady_clock::now();
}

template<typename T>
inline void print_buffer(T& buffer, std::size_t len) {
    std::size_t printed = 0;

    for (auto& elem: buffer) {
        if (printed >= len) {
            break;
        }
        std::cerr << std::hex << (unsigned int)(elem) << ' ';
        printed++;
        if ((printed % 16) == 0) std::cerr << '\n';
    }

    if ((printed % 16) != 0) std::cerr << '\n';
}

#endif // __COMMON_H

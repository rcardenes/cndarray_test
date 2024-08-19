#include <chrono>
#include <cstdlib>
#include <string>
#include <vector>
#include <iostream>

#include <common.h>
#include <cxx_npy.h>
#include "multicast.hpp"


/*void Worker::loop() {*/
/*    size_t counter = 0;*/
/*    std::cout << "Size;FromRedis(µs);Parse(µs)\n";*/
/*    do {*/
/*        inner_sem.acquire();*/
/*        auto t1 = get_timestamp();*/
/*        if (!active) {*/
/*            std::cerr << "Processed " << counter << " arrays\n";*/
/*            break;*/
/*        }*/
/*        auto raw_array = _redis->get("arr");*/
/*        auto t2 = get_timestamp();*/
/*        counter++;*/
/*        bool fortran_order;*/
/*        size_t word_size;*/
/*        std::vector<size_t> shape;*/
/*        cnpy::parse_npy_header(reinterpret_cast<unsigned char*>((*raw_array).data()), word_size, shape, fortran_order);*/
/*        size_t dim = shape[0];*/
/*        auto t3 = get_timestamp();*/
/*        std::cout << dim << "x" << dim << ';'*/
/*                  << time_diff_us(t1, t2) << ';'*/
/*                  << time_diff_us(t2, t3) << '\n';*/
/*        outer_sem.release();*/
/*    } while(true);*/
/*}*/

int main() {
    auto receiving = true;
    size_t counter = 0;
    // Worker worker(&redis);


    // worker.loop();
    std::cout << "Received: " << counter << " notifications\n";

    return 0;
}

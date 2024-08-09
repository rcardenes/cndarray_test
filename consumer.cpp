#include <chrono>
#include <cstdlib>
#include <string>
#include <vector>
#include <iostream>
#include <semaphore>
#include <thread>

#include "common.h"
#include "cnpy.h"
#include "sw/redis++/redis++.h"

using namespace sw::redis;

class Worker {
public:
    Worker(Redis* redis);
    void read_array();
    void quit();
    void loop();

private:
    Redis *_redis;
    std::binary_semaphore inner_sem;
    std::binary_semaphore outer_sem;
    bool active;
};

Worker::Worker(Redis* redis)
    : _redis{redis},
      inner_sem{0},
      outer_sem{1},
      active{true}
{
};

void Worker::read_array() {
    if (outer_sem.try_acquire()) {
        inner_sem.release();
    }
}

void Worker::quit() {
    outer_sem.acquire();
    active = false;
    inner_sem.release();
}

void Worker::loop() {
    size_t counter = 0;
    std::cout << "Size;FromRedis(µs)\n";
    do {
        inner_sem.acquire();
        auto t1 = get_timestamp();
        if (!active) {
            std::cerr << "Processed " << counter << " arrays\n";
            break;
        }
        counter++;
        auto raw_array = _redis->get("arr");
        auto t2 = get_timestamp();
        bool fortran_order;
        size_t word_size;
        std::vector<size_t> shape;
        cnpy::parse_npy_header(reinterpret_cast<unsigned char*>((*raw_array).data()), word_size, shape, fortran_order);
        size_t dim = shape[0];
        std::cout << dim << "x" << dim << ';'
                  << time_diff_us(t1, t2) << '\n';
        outer_sem.release();
    } while(true);
}

int main() {
    auto redis = Redis("tcp://localhost:6379");
    auto receiving = true;
    size_t counter = 0;
    Worker worker(&redis);

    auto sub = redis.subscriber();
    sub.on_message([&](std::string channel, std::string message) {
            if (channel == "__keyspace@0__:arr") {
                counter++;
                worker.read_array();
            } else if (channel == "__keyspace@0__:arr::done") {
                receiving = false;
                worker.quit();
            }
            });
    sub.subscribe("__keyspace@0__:arr");
    sub.subscribe("__keyspace@0__:arr::done");


    std::jthread t_receiver{[&]() {
        while (receiving) {
            try {
                sub.consume();
            } catch (const Error &err) {
                std::cerr << "sub: Exception!\n";
            }
        }
    }};

    worker.loop();
    std::cout << "Received: " << counter << " notifications\n";

    return 0;
}

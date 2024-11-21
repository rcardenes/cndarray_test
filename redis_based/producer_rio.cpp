#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <iterator>
#include <random>

#include "common.h"
#include <cxx_npy.h>
#include <hiredis/hiredis.h>

using std::chrono::operator""ms;

constexpr unsigned REPS = 100;

constexpr size_t MAX_DIM = 512;

int main() {
    std::random_device rd;
    std::ranlux24_base gen(rd());
    std::uniform_int_distribution<> indices(0, 2);
    std::normal_distribution<> values(-5000.0, 5000.0);

    auto ctx = redisConnect("localhost", 6379);

    if (ctx == nullptr || ctx->err) {
        if (ctx) {
            std::cerr << "Error: " << ctx->errstr << '\n';
        } else {
            std::cerr << "Can't allocate redis context\n";
        }
        return -1;
    }

    std::cout << "Size;Generation(µs);ToRedis(µs);Since start(µs)\n";

    unsigned dim = 512;
    unsigned buf_size = dim * dim * sizeof(float);
    float *buffer = new float[dim*dim];

    auto start_point = get_timestamp();
    auto next_point = start_point + 100ms;
    for(auto i=0; i < REPS; i++, next_point += 100ms) {
        auto t1 = get_timestamp();

        for (int i = 0; i < (dim * dim); i++)
            buffer[i] = values(gen);

        auto t2 = get_timestamp();

        auto reply = (redisReply*) redisCommand(ctx, "GARR32.SET arr 512 512 %b", buffer, buf_size);
        if (reply->type == REDIS_REPLY_ERROR) {
            std::cerr <<reply->str << "\n";
        }
        auto t3 = get_timestamp();
        std::cout << dim << 'x' << dim << ';'
                  << time_diff_us(t1, t2) << ';'
                  << time_diff_us(t2, t3) << ';'
                  << time_diff_us(start_point, t1) << '\n';
        freeReplyObject(reply);
        std::this_thread::sleep_until(next_point);
    }

    redisCommand(ctx, "SET arr::done 1");

    return 0;
}

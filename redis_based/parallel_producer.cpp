#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <iterator>
#include <random>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <cstdio>

#include "common.h"
#include "cxx_npy.h"
#include <hiredis/hiredis.h>

constexpr unsigned REPS = 1000;
constexpr size_t MAX_DIM = 512;
constexpr unsigned SHAPES[] = { 128, 256, 512 };
constexpr unsigned num_chunks[] = { 1, 4, 16 };
constexpr size_t NUM_THREADS = 16;
constexpr size_t CHUNK_SIZE = (128*128*sizeof(double));
constexpr size_t FIRST_CHUNK = npy::HEADER_SIZE + CHUNK_SIZE;

struct Shared {
    const size_t size;
    bool quit;
    std::vector<const char *> pointers;
    std::vector<std::mutex> control_mx;
    std::mutex var_mx;
    size_t working;
    std::condition_variable cv;

    Shared(size_t num)
        : size{num},
          quit{false},
          pointers{num},
          control_mx{num},
          working{false}
    {
        for (auto i = 0; i < num; i++) {
            control_mx[i].lock();
        }
    }

    void lock(size_t idx) {
        control_mx[idx].lock();
    }

    void awake(size_t idx) {
        control_mx[idx].unlock();
        inc_working();
    }

    void set_quit() {
        quit = true;
        for (auto i = 0; i < size; i++) {
            awake(i);
        }
    }

    void inc_working() {
        std::lock_guard<std::mutex> lock{var_mx};
        working++;
    }
    void dec_working() {
        std::unique_lock<std::mutex> lock{var_mx};
        working--;
        if (working == 0) {
            cv.notify_all();
        }

        lock.unlock();
    }

    const char *get_pointer(size_t idx) {
        return pointers[idx];
    }

    void wait_idle() {
        std::unique_lock<std::mutex> lock{var_mx};

        while (working > 0) {
            cv.wait(lock);
        }

        lock.unlock();
    }
};

void worker_loop(
        const size_t idx,
        const size_t chunk_size,
        Shared *sh,
        const char *host,
        int port)
{
    size_t cmd_fmt_size = snprintf(nullptr, 0, "SET arr.%lu %%b", idx);
    char cmd_fmt[cmd_fmt_size];
    sprintf(cmd_fmt, "SET arr.%lu %%b", idx);

    auto ctx = redisConnect(host, port);
    while (true) {
        sh->lock(idx);
        if (sh->quit) {
            break;
        }
        auto ptr = sh->get_pointer(idx);
        redisCommand(ctx, cmd_fmt, ptr, chunk_size);
        sh->dec_working();
    }
}

const char *HOST = "localhost";
const int PORT = 6379;

int main() {
    std::random_device rd;
    std::ranlux24_base gen(rd());
    std::uniform_int_distribution<> indices(0, 2);
    std::normal_distribution<> values(-5000.0, 5000.0);

    auto ctx = redisConnect(HOST, PORT);

    if (ctx == nullptr || ctx->err) {
        if (ctx) {
            std::cerr << "Error: " << ctx->errstr << '\n';
        } else {
            std::cerr << "Can't allocate redis context\n";
        }
        return -1;
    }

    std::vector<std::vector<char>> templates;
    for (auto dim: SHAPES) {
        templates.push_back(npy::generate_template_array(dim));
    }

    Shared sh{NUM_THREADS};

    std::vector<std::thread> worker_pool;
    for (auto i = 0; i < NUM_THREADS; i++) {
        size_t chunk_size = i == 0 ? FIRST_CHUNK : CHUNK_SIZE;
        worker_pool.emplace_back(worker_loop, i, chunk_size, &sh, HOST, PORT);
    }

    std::cout << "Size;Generation(µs);Serialization(µs);ToRedis(µs)\n";
    std::vector<double> data(MAX_DIM*MAX_DIM);

    for(auto i = 0; i < REPS; i++) {
        auto t1 = get_timestamp();
        auto idx = indices(gen);
        unsigned dim = SHAPES[idx];
        unsigned nchunks = num_chunks[idx];
        auto& buffer = templates[idx];

        for (int i = 0; i < (dim * dim); i++)
            data[i] = values(gen);

        auto t2 = get_timestamp();
        npy::serialize_array(data, buffer);
        auto t3 = get_timestamp();

        const char *raw_buffer = buffer.data();
        sh.pointers[0] = raw_buffer;
        sh.awake(0);

        for (size_t i = 1, pos = FIRST_CHUNK; i < nchunks; i++, pos += CHUNK_SIZE) {
            sh.pointers[i] = raw_buffer + pos;
            sh.awake(i);
        }

        sh.wait_idle();

        auto t4 = get_timestamp();
        std::cout << dim << 'x' << dim << ';'
                  << time_diff_us(t1, t2) << ';'
                  << time_diff_us(t2, t3) << ';'
                  << time_diff_us(t3, t4) << '\n';
    }

    sh.set_quit();

    for (auto i = 0; i < NUM_THREADS; i++ ) {
        worker_pool[i].join();
    }

    redisCommand(ctx, "SET arr::done 1");

    return 0;
}

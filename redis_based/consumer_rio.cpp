#include <chrono>
#include <cstdlib>
#include <string>
#include <iostream>
#include <thread>

#if __cplusplus >= 201907L
#include <semaphore>
#else
#include "semaphore.h"
#endif

#include "common.h"
#include <hiredis/async.h>
#include <hiredis/adapters/poll.h>

class Worker {
public:
    Worker(redisContext*);
    void read_array();
    void quit();
    void loop();
    bool done();

private:
    redisContext *ctx;
    std::binary_semaphore inner_sem;
    std::binary_semaphore outer_sem;
    bool active;
};

Worker::Worker(redisContext* ctx)
    : ctx(ctx),
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

bool Worker::done() {
    return !active;
}

void Worker::loop() {
    size_t counter = 0;
    std::cout << "Size;FromRedis(µs);Parse(µs)\n";
    do {
        inner_sem.acquire();
        auto t1 = get_timestamp();
        if (!active) {
            std::cerr << "Processed " << counter << " arrays\n";
            break;
        }
        auto reply = (redisReply*)redisCommand(ctx, "GARR32.GET arr");
        if (reply->type == REDIS_REPLY_ERROR) {
            std::cerr << reply->str << "\n";
            freeReplyObject(reply);
            continue;
        } else if (reply->type == REDIS_REPLY_ARRAY) {
            std::cout << reply->elements << " elements array\n";
        } else {
            std::cerr << "Wrong reply type\n";
            freeReplyObject(reply);
            continue;
        }
        /*auto t2 = get_timestamp();*/
        /*counter++;*/
        /*bool fortran_order;*/
        /*size_t word_size;*/
        /*std::vector<size_t> shape;*/
        /*cnpy::parse_npy_header(reinterpret_cast<unsigned char*>((*raw_array).data()), word_size, shape, fortran_order);*/
        /*size_t dim = shape[0];*/
        /*auto t3 = get_timestamp();*/
        /*std::cout << dim << "x" << dim << ';'*/
        /*          << time_diff_us(t1, t2) << ';'*/
        /*          << time_diff_us(t2, t3) << '\n';*/
        outer_sem.release();
    } while(true);
}

struct Subscriber {
    Worker *worker;
    size_t counter;
    bool quit;
    std::binary_semaphore connected;
};

void processArrNotification(redisAsyncContext *ac, void *reply, void*) {
    redisReply *r = (redisReply *)reply;
    if (r->type != REDIS_REPLY_ARRAY) {
        std::cerr << "Something wrong with the arr notification\n";
    } else {
        Subscriber * sub= (Subscriber *)ac->data;
        auto msg_type = std::string{((redisReply*)r->element[0])->str};
        if (msg_type == "message") {
            sub->counter++;
            sub->worker->read_array();
        }
    }
}

void processArrDoneNotification(redisAsyncContext *ac, void *reply, void*) {
    redisReply *r = (redisReply *)reply;
    if (r->type != REDIS_REPLY_ARRAY) {
        std::cerr << "Something wrong with the arr notification\n";
    } else {
        Subscriber * sub= (Subscriber *)ac->data;
        auto msg_type = std::string{((redisReply*)r->element[0])->str};
        if (msg_type == "message") {
            sub->worker->quit();
        }
    }
}

int main() {
    auto receiving = true;
    size_t counter = 0;
    auto ctx = redisConnect("localhost", 6379);
    auto actx = redisAsyncConnect("localhost", 6379);

    Worker worker(ctx);
    Subscriber subscriber {
        &worker,
        0,
        false,
        std::binary_semaphore{0}
    };

    actx->data = (void*)&subscriber;


    if (actx->err) {
        std::cerr << "Error " << actx->errstr << "\n";
        return 1;
    }

    redisPollAttach(actx);
    redisAsyncSetConnectCallback(actx, [](const redisAsyncContext *ac, int status) {
        Subscriber *sub = (Subscriber *)ac->data;
        if (status != REDIS_OK) {
            std::cerr << "Error: " << ac->errstr << '\n';
            sub->worker->quit();
            sub->quit = true;
        }

        sub->connected.release();
    });
    redisAsyncSetDisconnectCallback(actx, [](const redisAsyncContext *ac, int status) {
        if (status != REDIS_OK) {
            std::cerr << "Error: " << ac->errstr << '\n';
        }
        ((Worker *)ac->data)->quit();
    });

    // Wait until connected
    while (!subscriber.connected.try_acquire()) {
        redisPollTick(actx, 0.1);
    }
    // If quit flag is raised, an error occurred
    if (subscriber.quit) {
        return 1;
    }
    /*        if (channel == "__keyspace@0__:arr") {*/
    /*            counter++;*/
    /*            worker.read_array();*/
    /*        } else if (channel == "__keyspace@0__:arr::done") {*/
    /*            receiving = false;*/
    /*            worker.quit();*/
    /*        }*/
    /*        });*/

    redisAsyncCommand(actx, processArrNotification, NULL, "SUBSCRIBE __keyspace@0__:arr");
    redisAsyncCommand(actx, processArrDoneNotification, NULL, "SUBSCRIBE __keyspace@0__:arr::done");



    /*auto sub = redis.subscriber();*/
    /*sub.on_message([&](std::string channel, std::string message) {*/
    /*        if (channel == "__keyspace@0__:arr") {*/
    /*            counter++;*/
    /*            worker.read_array();*/
    /*        } else if (channel == "__keyspace@0__:arr::done") {*/
    /*            receiving = false;*/
    /*            worker.quit();*/
    /*        }*/
    /*        });*/

    /*std::thread t_receiver{[&]() {*/
    /*    redisReply *reply;*/
    /**/
    /*    while (receiving) {*/
    /*        try {*/
    /*            sub.consume();*/
    /*        } catch (const Error &err) {*/
    /*            std::cerr << "sub: Exception!\n";*/
    /*        }*/
    /*    }*/
    /*}};*/

    std::thread t_worker{[&]() {
        worker.loop();
    }};

    while (!worker.done()) {
        redisPollTick(actx, 0.1);
    }
    t_worker.join();
    std::cout << "Received: " << subscriber.counter << " notifications\n";

    return 0;
}

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <iterator>
#include <random>

#include "common.h"
#include <cxx_npy.h>
#include <hiredis/hiredis.h>

constexpr unsigned REPS = 1000;
constexpr unsigned SHAPES[] = { 128, 256, 512 };

constexpr size_t MAX_DIM = 512;

std::vector<unsigned char> read_file(std::string file_name) {
    std::ifstream npy_file(file_name, std::ios::binary | std::ios::ate);
    auto fsize = npy_file.tellg();
    npy_file.seekg(0);

    std::vector<unsigned char> contents;

    contents.assign(
            std::istreambuf_iterator<char>(npy_file),
            std::istreambuf_iterator<char>()
            );

    return contents;
}

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

    std::vector<std::vector<char>> templates;
    std::vector<std::string_view> views;
    for (auto dim: SHAPES) {
        templates.push_back(npy::generate_template_array(dim));
        auto&& templ = templates.back();
        views.emplace_back(templ.data(), templ.size());
    }

    std::cout << "Size;Generation(µs);Serialization(µs);ToRedis(µs)\n";
    std::vector<double> data(MAX_DIM*MAX_DIM);

    for(auto i=0; i < REPS; i++) {
        auto t1 = get_timestamp();
        auto idx = indices(gen);
        unsigned dim = SHAPES[idx];
        auto& buffer = templates[idx];
        auto& sv = views[idx];

        for (int i = 0; i < (dim * dim); i++)
            data[i] = values(gen);
            // data[i] = double(i);

        auto t2 = get_timestamp();
        npy::serialize_array(data, buffer);
        auto t3 = get_timestamp();

        redisCommand(ctx, "SET arr %b", sv.data(), sv.size());
        auto t4 = get_timestamp();
        std::cout << dim << 'x' << dim << ';'
                  << time_diff_us(t1, t2) << ';'
                  << time_diff_us(t2, t3) << ';'
                  << time_diff_us(t3, t4) << '\n';
    }

    redisCommand(ctx, "SET arr::done 1");

    return 0;
}

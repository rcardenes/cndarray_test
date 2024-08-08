#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <iterator>
#include <random>

#include "common.h"
#include <cnpy.h>
#include <sw/redis++/redis++.h>

using namespace sw::redis;

constexpr unsigned REPS = 100;
constexpr unsigned SHAPES[] = { 128, 256, 512, 1024, 2048 };

std::vector<char> create_npy(std::vector<double>& data, const std::vector<size_t>& shape) {
    auto array = cnpy::create_npy_header<double>(shape);
    for (auto elem: data) {
        const unsigned char* ptr = reinterpret_cast<const unsigned char *>(&elem);
        for (auto i = 0; i < sizeof(double); i++, ptr++) {
            array.push_back(*ptr);
        }
    }

    return array;
}

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
    std::uniform_int_distribution<> indices(0, 4);
    std::normal_distribution<> values(-5000.0, 5000.0);

    auto redis = Redis("tcp://localhost:6379");

    std::cerr << "Size;Generation(µs);ToString(µs);ToRedis(µs)\n";

    for(auto i=0; i < REPS; i++) {
        auto t1 = get_timestamp();
        auto idx = indices(gen);
        unsigned dim = SHAPES[idx];

        std::vector<double> data(dim * dim);
        for (int i = 0; i < (dim * dim); i++)
            data[i] = values(gen);

        std::vector<size_t> shape{ dim, dim };

        auto array = create_npy(data, shape);
        auto t2 = get_timestamp();

        auto str = std::string{array.begin(), array.end()};
        auto t3 = get_timestamp();

        redis.set("arr", str);
        auto t4 = get_timestamp();
        std::cerr << dim << 'x' << dim << ';'
                  << time_diff_us(t1, t2) << ';'
                  << time_diff_us(t2, t3) << ';'
                  << time_diff_us(t3, t4) << '\n';
    }

    return 0;
}

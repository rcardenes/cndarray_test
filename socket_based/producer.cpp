#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <iterator>
#include <random>
#include <algorithm>

#include <asio.hpp>

using asio::ip::udp;

#include "multicast.hpp"
#include <common.h>
#include <cxx_npy.h>

constexpr unsigned REPS = 1000;
constexpr unsigned SHAPES[] = { 128, 256, 512 };

constexpr size_t MAX_DIM = 512;

void send_vector(udp::socket& sock, udp::endpoint& endpoint, uint16_t dim, const std::vector<double>& vec) {
    std::string first_packet(FIRST_PACKET_SIZE, ' ');
    auto* inner = first_packet.data();

    first_packet.replace(0, MULTICAST_PACKET_HEADER.size(), MULTICAST_PACKET_HEADER);

    sock.send_to(asio::buffer(first_packet), endpoint);
    
}

int main() {
    std::random_device rd;
    std::ranlux24_base gen(rd());
    std::uniform_int_distribution<> indices(0, 2);
    std::normal_distribution<> values(-5000.0, 5000.0);

    auto mcast_endpoint = udp::endpoint{
        asio::ip::make_address(MULTICAST_GRP).to_v4(),
        MULTICAST_PORT
    };
    auto outbound_ip = asio::ip::make_address(MULTICAST_OUTBOUND).to_v4();

    asio::io_context io_context;
    udp::socket sock{io_context, udp::endpoint(udp::v4(), 0)};

    sock.set_option(asio::ip::multicast::hops(MULTICAST_TTL));
    sock.set_option(asio::ip::multicast::outbound_interface(outbound_ip));

    std::vector<std::vector<char>> templates;
    std::vector<std::string_view> views;
    for (auto dim: SHAPES) {
        templates.push_back(npy::generate_template_array(dim));
        auto&& templ = templates.back();
        views.emplace_back(templ.data(), templ.size());
    }

    std::cout << "Size;Generation(µs);Serialization(µs);ToRedis(µs)\n";
    std::vector<double> data(MAX_DIM*MAX_DIM);

    std::string_view foo_text{"foo bar"};

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

        send_vector(sock, mcast_endpoint, dim, data);
        auto t4 = get_timestamp();
        std::cout << dim << 'x' << dim << ';'
                  << time_diff_us(t1, t2) << ';'
                  << time_diff_us(t2, t3) << ';'
                  << time_diff_us(t3, t4) << '\n';
    }

    // redisCommand(ctx, "SET arr::done 1");

    return 0;
}

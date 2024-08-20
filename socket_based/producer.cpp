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

static uint8_t send_buffer[MULTICAST_PACKET_SIZE];

inline void encode_header(uint8_t* buffer, uint16_t dim) {
    auto dim_offset = MULTICAST_PACKET_HEADER.size();
    const uint8_t* dim_ptr = reinterpret_cast<uint8_t*>(&dim);

    memcpy(buffer, MULTICAST_PACKET_HEADER.data(), dim_offset);
    buffer[dim_offset] = *dim_ptr;
    buffer[dim_offset+1] = *(dim_ptr + 1);
}

void send_final(udp::socket& sock, udp::endpoint&endpoint) {
    encode_header(send_buffer, NO_MORE_PACKETS);

    sock.send_to(asio::buffer(send_buffer, FIRST_PACKET_SIZE), endpoint);
}

void send_vector(udp::socket& sock, udp::endpoint& endpoint, uint16_t dim, const std::vector<double>& vec) {
    std::size_t payload_size = (dim * dim) * sizeof(double);
    // The first 2 bytes are a packet counter
    std::size_t total_payload_packets = payload_size / (MULTICAST_PAYLOAD_SIZE);

    // std::cerr << "Sending " << dim << 'x' << dim << " array, " << total_payload_packets << " packets\n";

    encode_header(send_buffer, dim);

    sock.send_to(asio::buffer(send_buffer, FIRST_PACKET_SIZE), endpoint);

    uint16_t packet_counter = 0;
    const uint8_t* payload_ptr = reinterpret_cast<const uint8_t*>(vec.data());
    const char* pc_ptr = reinterpret_cast<char *>(&packet_counter);
    while (packet_counter < total_payload_packets) {
        send_buffer[0] = *pc_ptr;
        send_buffer[1] = *(pc_ptr + 1);
        memcpy(&send_buffer[2], payload_ptr, MULTICAST_PAYLOAD_SIZE);

        sock.send_to(asio::buffer(send_buffer, MULTICAST_PACKET_SIZE), endpoint);

        packet_counter++;
    }
    
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

    std::cout << "Size;Generation(µs);Sending(µs)\n";
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
        send_vector(sock, mcast_endpoint, dim, data);
        auto t3 = get_timestamp();
        std::cout << dim << 'x' << dim << ';'
                  << time_diff_us(t1, t2) << ';'
                  << time_diff_us(t2, t3) << '\n';
    }

    send_final(sock, mcast_endpoint);

    return 0;
}

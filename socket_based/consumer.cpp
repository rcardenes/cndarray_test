#include <chrono>
#include <cstdlib>
#include <string>
#include <array>
#include <iostream>
#include <ios>

#include <common.h>
#include <cxx_npy.h>
#include "multicast.hpp"

#include <asio.hpp>

using asio::ip::udp;

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

std::array<uint8_t, MULTICAST_PACKET_SIZE> recv_buffer;

template<typename T, std::size_t N>
bool validate_header(std::array<T, N>& buffer, uint16_t &dim) {
    if (buffer.size() < FIRST_PACKET_SIZE) {
        return false;
    }

    auto buffer_it = buffer.begin();
    auto header_it = MULTICAST_PACKET_HEADER.begin();

    while (header_it != MULTICAST_PACKET_HEADER.end()) {
        if ((buffer_it == buffer.end()) || (uint8_t(*buffer_it) != uint8_t(*header_it))) {
            std::cerr << "Corrupt packet header\n";
            std::cerr << std::hex << (unsigned int)(*buffer_it) << " != " << (unsigned int)(*header_it) << '\n';
            return false;
        }

        header_it++;
        buffer_it++;
    }

    dim = *reinterpret_cast<uint16_t*>(buffer_it);

    return true;
}

int main() {
    auto receiving = true;
    size_t counter = 0;

    auto mcast_ip = asio::ip::make_address(MULTICAST_GRP).to_v4();
    auto mcast_endpoint = udp::endpoint{mcast_ip, MULTICAST_PORT};
    auto listening_ip = asio::ip::make_address(MULTICAST_OUTBOUND).to_v4();

    asio::io_context io_context;

    udp::socket sock{io_context, mcast_endpoint.protocol()};

    // So that we can have multiple clients listening
    sock.set_option(asio::socket_base::reuse_address(true));
    // This sets IP_MULTICAST_IF
    sock.set_option(asio::ip::multicast::outbound_interface(listening_ip));
    sock.set_option(asio::ip::multicast::join_group(mcast_ip));

    sock.bind(mcast_endpoint);

    std::size_t corrupt_count = 0;
    std::size_t unexpected_count = 0;
    std::size_t missed_count = 0;
    uint16_t dim;

    std::cout << "Size;Validation(µs)\n";

    do {
        udp::endpoint sender_endpoint;

        auto received_size = sock.receive_from(asio::buffer(recv_buffer.data(), MULTICAST_PACKET_SIZE), sender_endpoint);

        auto t1 = get_timestamp();

        if (received_size != FIRST_PACKET_SIZE) {
            std::cerr << "Unexpected " << received_size << " sized packet\n";
            unexpected_count++;
            continue;
        }

        if (!validate_header(recv_buffer, dim)) {
            std::cerr << "Corrupt header\n";
            corrupt_count++;
            continue;
        }

        if (dim == NO_MORE_PACKETS)
            break;

        std::size_t total_packets = (dim * dim * sizeof(double)) / MULTICAST_PAYLOAD_SIZE;
        std::size_t next_packet = 0;
        // std::cerr << "Receiving an " << dim << 'x' << dim << " array, " << total_packets << " packets\n";

        uint16_t packet_num;
        while (next_packet < total_packets) {
            auto payload_size = sock.receive_from(asio::buffer(recv_buffer.data(), MULTICAST_PACKET_SIZE), sender_endpoint);
            if (payload_size != MULTICAST_PACKET_SIZE) {
                std::cerr << "Expected payload packet, got something " << payload_size << " sized instead\n";
                unexpected_count++;
                break;
            }

            packet_num = *reinterpret_cast<const uint16_t*>(recv_buffer.data());
            if (packet_num != next_packet) {
                missed_count += packet_num - next_packet;
                next_packet = packet_num + 1;
            } else {
                next_packet++;
            }
        }
        auto t2 = get_timestamp();

        std::cout << dim << "x" << dim << ';'
                  << time_diff_us(t1, t2) << '\n';

    } while(true);

    std::cerr << "Counters: "
        << corrupt_count << " corrupt / "
        << unexpected_count << " unexpected / "
        << missed_count << " missed\n";

    return 0;
}

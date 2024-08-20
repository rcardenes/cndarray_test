#ifndef __MULTICAST_HPP__
#define __MULTICAST_HPP__

#include <algorithm>
#include <string_view>
#include <string>
#include <vector>

constexpr short MULTICAST_PORT = 30000;
constexpr std::string_view MULTICAST_GRP{"239.1.2.3"};
constexpr unsigned MULTICAST_TTL = 2;
constexpr std::string_view MULTICAST_OUTBOUND{"172.26.70.99"};
constexpr size_t MULTICAST_PAYLOAD_SIZE = 32768;
constexpr size_t MULTICAST_PACKET_SIZE = MULTICAST_PAYLOAD_SIZE + 2;
constexpr std::string_view MULTICAST_PACKET_HEADER{"\xff\xfeMCASTPKT"};
constexpr size_t FIRST_PACKET_SIZE = MULTICAST_PACKET_HEADER.size() + sizeof(uint16_t);

constexpr uint16_t NO_MORE_PACKETS = 0xffff;

struct MulticastHeader {
    std::vector<char> _data_source;

    MulticastHeader(std::vector<char>& data_source) : _data_source{data_source} {}

    bool is_valid() const {
        if (_data_source.size() != FIRST_PACKET_SIZE)
            return false;

        return std::lexicographical_compare(
                MULTICAST_PACKET_HEADER.begin(), MULTICAST_PACKET_HEADER.end(),
                _data_source.begin(), _data_source.begin() + MULTICAST_PACKET_HEADER.size());
    }

    uint16_t get_dim() const {
        const char *ptr = _data_source.data() + MULTICAST_PACKET_HEADER.size();
        return *reinterpret_cast<const uint16_t*>(ptr);
    }

    std::string get_npy_header() const {
        return std::string{_data_source.begin() + MULTICAST_PACKET_HEADER.size(), _data_source.end()};
    }
};


#endif // __MULTICAST_HPP__

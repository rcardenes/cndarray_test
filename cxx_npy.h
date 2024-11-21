#ifndef __CXX_NPY__
#define __CXX_NPY__

/*
 * Specialized NPY library focusing on square arrays of double
 */

#include <vector>
#include <cstdio>
#include <cstring>
#include <string_view>
#include <cstdint>

// "\x93NUMPY\x01\x00v\x00{'descr': '<f8', 'fortran_order': False, 'shape': (20, 3), }                                                         \n"

namespace npy {
    constexpr std::string_view header{"\x93NUMPY\x01\x00v\x00", 10};
    constexpr std::string_view header_payload_template{"{'descr': '<%s', 'fortran_order': False, 'shape': (%ld, %ld), }"};

    // Enough to contain the header plus padding (Npy requires padding to multiples
    // of 64 bytes
    constexpr size_t HEADER_SIZE{128};

    template<typename T>
    constexpr const char* get_type_format() = delete;
    template<>
    constexpr const char* get_type_format<uint16_t>() { return "i2"; };
    template<>
    constexpr const char* get_type_format<uint32_t>() { return "i4"; };
    template<>
    constexpr const char* get_type_format<float>() { return "f4"; };
    template<>
    constexpr const char* get_type_format<double>() { return "f8"; };

    template<typename T>
    std::vector<char> generate_template_array(size_t dim) {
        auto dtype = get_type_format<T>();
        int total_size = HEADER_SIZE + (dim * dim * sizeof(T));

        std::vector<char> buf(total_size, ' ');
        std::copy(header.begin(), header.end(), buf.begin());
        auto offset = header.size();
        int text_len = std::sprintf(buf.data() + offset, header_payload_template.data(), dtype, dim, dim);
        buf[offset + text_len] = ' ';
        buf[HEADER_SIZE - 1] = '\n';

        return buf;
    }

    template<typename T>
    void serialize_array(std::vector<T>& data, std::vector<char>& buffer) {
        auto buffer_begin = &(*buffer.begin());
        auto data_begin = &(*data.begin());
        memcpy(buffer_begin + HEADER_SIZE, data_begin, buffer.size() - HEADER_SIZE);
    }

    template<typename T>
    void deserialize_array(std::vector<T>& data, std::vector<char>& buffer) {
        auto buffer_begin = &(*buffer.begin());
        auto data_begin = &(*data.begin());
        memcpy(data_begin, buffer_begin + HEADER_SIZE, buffer.size() - HEADER_SIZE);
    }

}

#endif // __CXX_NPY__

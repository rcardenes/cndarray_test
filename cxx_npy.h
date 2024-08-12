#ifndef __CXX_NPY__
#define __CXX_NPY__

/*
 * Specialized NPY library focusing on square arrays of double
 */

#include <vector>
#include <cstdio>
#include <cstring>
#include <string_view>

// "\x93NUMPY\x01\x00v\x00{'descr': '<f8', 'fortran_order': False, 'shape': (20, 3), }                                                         \n"

namespace npy {
    constexpr std::string_view header{"\x93NUMPY\x01\x00v\x00", 10};
    constexpr std::string_view header_payload_template{"{'descr': '<f8', 'fortran_order': False, 'shape': (%d, %d), }"};

    // Enough to contain the header plus padding (Npy requires padding to multiples
    // of 64 bytes
    constexpr size_t HEADER_SIZE{128};

    inline
    std::vector<char> generate_template_array(size_t dim) {
        int total_size = HEADER_SIZE + (dim * dim * 8);

        std::vector<char> buf(total_size, ' ');
        std::copy(header.begin(), header.end(), buf.begin());
        auto offset = header.size();
        int text_len = std::sprintf(buf.data() + offset, header_payload_template.data(), dim, dim);
        buf[offset + text_len] = ' ';
        buf[HEADER_SIZE - 1] = '\n';

        return buf;
    }

    inline
    void serialize_array(std::vector<double>& data, std::vector<char>& buffer) {
        auto buffer_begin = &(*buffer.begin());
        auto data_begin = &(*data.begin());
        memcpy(buffer_begin + HEADER_SIZE, data_begin, buffer.size() - HEADER_SIZE);
    }

    inline
    void deserialize_array(std::vector<double>& data, std::vector<char>& buffer) {
        auto buffer_begin = &(*buffer.begin());
        auto data_begin = &(*data.begin());
        memcpy(data_begin, buffer_begin + HEADER_SIZE, buffer.size() - HEADER_SIZE);
    }

}

#endif // __CXX_NPY__

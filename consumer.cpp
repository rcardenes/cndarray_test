#include "cnpy.h"
#include <string>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <iterator>

constexpr int Nx = 3;
constexpr int Ny = 2;

std::vector<unsigned char> read_file(std::string file_name) {
    std::ifstream npy_file(file_name, std::ios::binary | std::ios::ate);
    auto fsize = npy_file.tellg();
    npy_file.seekg(0);

    std::vector<unsigned char> contents;
//    contents.reserve(fsize);
    contents.assign(
            std::istreambuf_iterator<char>(npy_file),
            std::istreambuf_iterator<char>()
            );

    return contents;
}

int main() {
    /*srand(0);*/
    /*std::vector<double> data(Nx * Ny);*/
    /**/
    /*for (int i = 0; i < Nx*Ny; i++) data[i] = rand();*/
    /**/
    /*cnpy::npy_save("arr1.npy", &data[0], {Ny, Nx}, "w");*/

    auto buffer = read_file("arr1.npy");
    auto array = cnpy::npy_load(buffer);

    std::cout << "Read a " << array.shape[0] << "x" << array.shape[1] << " array\n";

    return 0;
}

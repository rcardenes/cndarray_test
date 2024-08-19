cmake_minimum_required(VERSION 3.24 FATAL_ERROR)
project(Npy_Testing
        VERSION 0.2
        LANGUAGES CXX)

set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
link_libraries(Threads::Threads)

# Grab dependencies
include(FetchContent)
FetchContent_Declare(
    hiredis
    GIT_REPOSITORY https://github.com/redis/hiredis.git
    GIT_TAG v1.2.0
    FIND_PACKAGE_ARGS NAMES hiredis
)
FetchContent_MakeAvailable(hiredis)
find_path(HIREDIS_INCLUDE hiredis)
find_library(HIREDIS_LIB hiredis)

find_package(redis++ REQUIRED)
find_path(REDIS_PLUS_PLUS_INCLUDE_DIR sw)
find_library(REDIS_PLUS_PLUS_LIB redis++)

find_path(CNPY_INCLUDE_DIR cnpy.h)
find_library(CNPY_LIB cnpy)

find_path(ASIO_INCLUDE_DIR asio.hpp)

include_directories(
    "${PROJECT_SOURCE_DIR}"
    "${HIREDIS_INCLUDE}"
    "${REDIS_PLUS_PLUS_INCLUDE_DIR}"
    "${CNPY_INCLUDE_DIR}"
    "${ASIO_INCLUDE_DIR}"
)
link_libraries(${HIREDIS_LIB} ${REDIS_PLUS_PLUS_LIB} ${CNPY_LIB})

add_executable(redis_producer redis_based/producer.cpp)
add_executable(redis_parallel_producer redis_based/parallel_producer.cpp)
add_executable(redis_consumer redis_based/consumer.cpp)
add_executable(socket_producer socket_based/producer.cpp)
add_executable(socket_consumer socket_based/consumer.cpp)

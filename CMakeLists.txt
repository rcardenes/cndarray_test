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

include_directories("${PROJECT_SOURCE_DIR}" "${HIREDIS_INCLUDE}" "${REDIS_PLUS_PLUS_INCLUDE_DIR}" "${CNPY_INCLUDE_DIR}")
link_libraries(${HIREDIS_LIB} ${REDIS_PLUS_PLUS_LIB} ${CNPY_LIB})

add_executable(producer producer.cpp)
add_executable(consumer consumer.cpp)

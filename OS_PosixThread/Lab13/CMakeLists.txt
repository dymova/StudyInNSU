cmake_minimum_required(VERSION 3.3)
project(Lab13)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lpthread -lrt -pthread")

set(SOURCE_FILES main.cpp)
add_executable(Lab13 ${SOURCE_FILES})
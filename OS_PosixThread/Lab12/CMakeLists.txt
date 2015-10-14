cmake_minimum_required(VERSION 3.3)
project(Lab12)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lpthread -pthread")

set(SOURCE_FILES main.cpp)
add_executable(Lab12 ${SOURCE_FILES})
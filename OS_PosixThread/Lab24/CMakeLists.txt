cmake_minimum_required(VERSION 3.3)
project(Lab24)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lpthread -pthread ")

set(SOURCE_FILES main.cpp myQueue.cpp myQueue.h)
add_executable(Lab24 ${SOURCE_FILES})
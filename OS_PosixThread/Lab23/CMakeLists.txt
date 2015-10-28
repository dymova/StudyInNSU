cmake_minimum_required(VERSION 3.3)
project(Lab23)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lpthread -pthread -lrt")

set(SOURCE_FILES main.cpp myQueue.h myQueue.cpp myQueue.h)
add_executable(Lab23 ${SOURCE_FILES})
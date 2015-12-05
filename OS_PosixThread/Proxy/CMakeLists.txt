cmake_minimum_required(VERSION 3.3)
project(Proxy)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++98")

set(SOURCE_FILES main.cpp Proxy.cpp Proxy.h ClientConnection.cpp ClientConnection.h ServerConnection.cpp ServerConnection.h CacheBucket.cpp CacheBucket.h)
add_executable(Proxy ${SOURCE_FILES})
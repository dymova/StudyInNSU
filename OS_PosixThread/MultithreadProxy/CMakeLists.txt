cmake_minimum_required(VERSION 3.3)
project(MultithreadProxy)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++98 -lpthread -pthread")

set(SOURCE_FILES main.cpp Proxy.cpp Proxy.h ClientConnection.cpp ClientConnection.h CacheBucket.cpp CacheBucket.h ServerConnection.cpp ServerConnection.h ThreadsBodies.cpp ThreadsBodies.h CacheStorage.cpp CacheStorage.h)
add_executable(MultithreadProxy ${SOURCE_FILES})
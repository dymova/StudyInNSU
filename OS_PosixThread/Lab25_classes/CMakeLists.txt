cmake_minimum_required(VERSION 3.3)
project(Lab25)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp Connection.cpp Connection.h Forwarder.cpp Forwarder.h)
#set(SOURCE_FILES main.cpp Server.cpp Server.h)
add_executable(Lab25 ${SOURCE_FILES})
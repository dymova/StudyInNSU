cmake_minimum_required(VERSION 3.3)
project(Lab16)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -lpthread -pthread")

set(SOURCE_FILES main.c)
add_executable(Lab16 ${SOURCE_FILES})
cmake_minimum_required(VERSION 3.1)
project(Task34_1)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

add_executable(cunsumer cunsumer.c)

add_executable(producer producer.c)
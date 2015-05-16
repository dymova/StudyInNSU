cmake_minimum_required(VERSION 3.1)
project(Task30)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

add_executable(master master.c Message.h)

add_executable(send send.c Message.h)
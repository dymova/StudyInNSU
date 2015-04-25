cmake_minimum_required(VERSION 3.1)
project(Task7)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

set(SOURCE_FILES main.c)
add_executable(Task7 ${SOURCE_FILES})
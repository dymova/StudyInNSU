cmake_minimum_required(VERSION 3.1)
project(Task16)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

set(SOURCE_FILES main.c)
add_executable(Task16 ${SOURCE_FILES})
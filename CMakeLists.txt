cmake_minimum_required(VERSION 3.12)
project(teddy C)

set(CMAKE_C_STANDARD 11)

add_executable(teddy main.c teddymap.h)
target_link_libraries(teddy m)
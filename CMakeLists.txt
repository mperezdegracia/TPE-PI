cmake_minimum_required(VERSION 3.21)
project(TPEFINAL C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-Wall -pedantic -std=c99 -lm -fsanitize=address")
add_executable(TPEFINAL main.c PeatonesADT.c)

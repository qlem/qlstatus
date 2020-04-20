cmake_minimum_required(VERSION 3.15)
project(qlstatus C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-W -Wall -Wextra -Werror")

add_executable(qlstatus
    src/main.c 
    src/battery.c
    src/int_to_str.c 
    src/alloc_buffer.c 
    src/qlstatus.h)

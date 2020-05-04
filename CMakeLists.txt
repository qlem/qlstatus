cmake_minimum_required(VERSION 3.15)
project(qlstatus C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-W -Wall -Wextra -Werror")

add_executable(qlstatus
    src/main.c
    src/config.c
    src/battery.c
    src/brightness.c
    src/cpu_temp.c
    src/cpu_usage.c
    src/wireless.c
    src/memory.c
    src/volume.c
    src/format.c
    src/to_str.c
    src/read_file.c
    src/read_dir.c
    src/common.c
    src/regex.c)

include_directories(inc)
target_include_directories(qlstatus PRIVATE /usr/include/libnl3)
target_link_libraries(qlstatus -lpthread -lnl-3 -lnl-genl-3 -lpulse)

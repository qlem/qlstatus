cmake_minimum_required(VERSION 3.15)
project(qlstatus C)

set(RELEASE "v1.1.8")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-W -Wall -Wextra -Werror -O2")

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBNOTIFY REQUIRED libnotify)
pkg_check_modules(LIBNL REQUIRED libnl-3.0 libnl-genl-3.0)

add_executable(qlstatus
    src/main.c
    src/time.c
    src/config.c
    src/battery.c
    src/brightness.c
    src/temperature.c
    src/cpu.c
    src/freq.c
    src/wireless.c
    src/memory.c
    src/pulse.c
    src/format.c
    src/to_str.c
    src/read_file.c
    src/read_dir.c
    src/common.c
    src/regex.c
    src/notify.c)

target_compile_definitions(qlstatus PRIVATE VERSION="${RELEASE}")
target_include_directories(qlstatus PRIVATE inc ${LIBNL_INCLUDE_DIRS} ${LIBNOTIFY_INCLUDE_DIRS})
target_link_libraries(qlstatus -lpthread -lpulse ${LIBNL_LINK_LIBRARIES} ${LIBNOTIFY_LINK_LIBRARIES})

install(CODE "
        execute_process(COMMAND ${CMAKE_SOURCE_DIR}/manual.sh
                        OUTPUT_FILE ${CMAKE_BINARY_DIR}/qlstatus.1
                        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
        message(STATUS \"Generating manual page\")")
install(FILES ${CMAKE_BINARY_DIR}/qlstatus.1 DESTINATION share/man/man1)
install(FILES qlstatus.conf TYPE SYSCONF)
install(TARGETS qlstatus RUNTIME DESTINATION bin)

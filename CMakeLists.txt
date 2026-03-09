cmake_minimum_required(VERSION 3.10)
project(green_thread C)
set(CMAKE_ASM_SOURCE_FILE_EXTENSIONS ${CMAKE_ASM_SOURCE_FILE_EXTENSIONS} S)
enable_language(ASM)
add_compile_definitions(_GNU_SOURCE)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_library(co_context STATIC
        src/cocontrol.S
        src/dlist.c
        src/scheduler.c
        src/channel.c
        include/channel.h
)
target_compile_options(co_context PRIVATE
        -Wall -Wextra -O2 -g
        -fno-stack-protector
)

target_include_directories(co_context PUBLIC include)
target_compile_options(co_context PRIVATE -Wall -Wextra -O2 -g)

add_executable(test_dlist

        src/test/test_dlist.c)
add_executable(test_scheduler

        src/test/test_scheduler.c)

target_link_libraries(test_dlist PRIVATE co_context)
target_link_libraries(test_scheduler PRIVATE co_context)
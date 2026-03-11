#环境设置
cmake_minimum_required(VERSION 3.10)
project(green_thread C)
set(CMAKE_ASM_SOURCE_FILE_EXTENSIONS ${CMAKE_ASM_SOURCE_FILE_EXTENSIONS} S)
enable_language(ASM)
add_compile_definitions(_GNU_SOURCE)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#lib
add_library(co_context STATIC
        src/cocontrol.S
        src/dlist.c
        src/scheduler.c
        src/channel.c
        include/channel.h
        src/test/utils.c
        src/test/utils.h
        src/slist.c
        include/slist.h
        src/hash_map.c
        include/hash_map.h
)
#编译选项
target_compile_options(co_context PRIVATE
        -Wall -Wextra -O2 -g
        -fno-stack-protector
)

target_include_directories(co_context PUBLIC include)
target_compile_options(co_context PRIVATE -Wall -Wextra -O2 -g)

#test
add_executable(test_dlist

        src/test/test_dlist.c)
add_executable(test_scheduler
        src/test/test_scheduler.c)
add_executable(test_channel src/test/test_channel.c
)

add_executable(test_slist src/test/test_slist.c)
#链接库
target_link_libraries(test_dlist PRIVATE co_context)
target_link_libraries(test_scheduler PRIVATE co_context)
target_link_libraries(test_channel PRIVATE co_context)
target_link_libraries(test_slist PRIVATE co_context)


cmake_minimum_required(VERSION 3.16)
project(TASK1)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)

add_executable(prog1 Prog1/prog1.cpp common.h)
add_executable(prog2 Prog2/prog2.cpp common.h)

target_link_libraries(prog1 Threads::Threads)
target_link_libraries(prog2 Threads::Threads)
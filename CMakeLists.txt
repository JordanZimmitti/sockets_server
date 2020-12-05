cmake_minimum_required(VERSION 3.17)
project(program_06_server)

set(CMAKE_CXX_STANDARD 14)

add_executable(
        program_06_server
        main.cpp shared.h
        Socket.cpp
        Socket.hpp
        tools.cpp
        tools.hpp
        Mom.cpp Mom.hpp)
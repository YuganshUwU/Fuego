cmake_minimum_required(VERSION 3.20)

project(fuego)

set(CMAKE_CXX_STANDARD 20)

add_executable(fue src/main.cpp
        src/tokenization.hpp
        src/parser.hpp
        src/generation.hpp
        src/arena.hpp)
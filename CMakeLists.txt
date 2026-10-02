cmake_minimum_required(VERSION 3.20)
project(kata CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(kata src/main.cpp
        src/tokenizer.h
        src/parser.h
        src/generation.h
        src/sugar.h
        src/arena.h)
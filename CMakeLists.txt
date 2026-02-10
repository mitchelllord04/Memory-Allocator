cmake_minimum_required(VERSION 4.0)
project(Malloc C)

set(CMAKE_C_STANDARD 11)

include_directories(include)
include_directories(tests)

add_executable(Malloc
        include/allocator.h
        src/allocator.c
        tests/test_allocator.c
        tests/unity.c
        tests/unity.h
        tests/unity_internals.h
        tests/test_allocator.c
        tests/test_allocator.c
        tests/test_alignment.c
        tests/test_split.c
        tests/test_coalesce.c)

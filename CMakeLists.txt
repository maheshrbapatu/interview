cmake_minimum_required(VERSION 3.26)
project(multi_threading)

set(CMAKE_CXX_STANDARD 17)

add_executable(multi_threading main.cpp
        reader_writer.cpp
        read_priority.cpp
        mutex_ex.cpp
        emplace_pushback.cpp
        shared_unique_lock.cpp
        scoped_lock.cpp
        lock_guard.cpp
        writer_priority.cpp)

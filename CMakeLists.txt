cmake_minimum_required(VERSION 3.22.1)

project(Aecs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(include)

add_definitions("-W")

add_executable(exec main.cpp)
target_link_libraries(exec PUBLIC aecslib)
cmake_minimum_required(VERSION 3.17)
project(ArgParser)

set(CMAKE_CXX_STANDARD 17)
include_directories(argParser)

add_executable(ArgParser main.cpp)
target_link_libraries(ArgParser fmt)
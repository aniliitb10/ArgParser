cmake_minimum_required(VERSION 3.16)
project(ArgParser)

set(CMAKE_CXX_STANDARD 14)
include_directories(include)

file(GLOB gTestSource "test/argParser/*.cpp")

add_executable(ArgParser main.cpp ${gTestSource} test/TestUtilTest.cpp)
target_link_libraries(ArgParser fmt gtest pthread)
cmake_minimum_required(VERSION 3.16)
project(argParser)

set(CMAKE_CXX_STANDARD 17)
include_directories(include)

add_executable(argParser main.cpp test/argParser/ArgParserTest.cpp test/TestUtilTest.cpp)
target_link_libraries(argParser gtest pthread)
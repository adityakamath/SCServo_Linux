cmake_minimum_required(VERSION 3.5)
set(project "RegWritePosition")
project(${project})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3")

include_directories(../../../src)
link_directories(../../../lib)
link_libraries(libSCServo.a)

file(GLOB hdrs *h)
file(GLOB srs *.cpp)

add_executable(${project} ${hdrs} ${srs})
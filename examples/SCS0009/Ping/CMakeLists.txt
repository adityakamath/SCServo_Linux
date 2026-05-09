cmake_minimum_required(VERSION 3.5)
set(project "Ping")
project(${project})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options(-O3)

include_directories(../../../include)
link_directories(../../../build)
link_libraries(libSCServo.a)

file(GLOB hdrs *h)
file(GLOB srs *.cpp)

add_executable(${project} ${hdrs} ${srs})

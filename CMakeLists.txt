
cmake_minimum_required(VERSION 3.5)
set(project "SCServo")
project(${project} LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O3")

# Only include SMSCL, SMS_STS, SCSerial, SCS, and INST files
set(hdrs SMSCL.h SMS_STS.h SCSerial.h SCS.h SCSCL.h INST.h)
set(srs SMSCL.cpp SMS_STS.cpp SCSerial.cpp SCS.cpp SCSCL.cpp)

add_library(${project} STATIC ${hdrs} ${srs})
set_target_properties(${project} PROPERTIES POSITION_INDEPENDENT_CODE ON)
#add_executable(${project} main.cpp ${hdrs} ${srs})

# --- Python bindings with nanobind ---
find_package(Python 3.8 REQUIRED COMPONENTS Interpreter Development.Module)
add_subdirectory(extern/nanobind)

nanobind_add_module(scservo_python src/bindings.cpp)
target_include_directories(scservo_python PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(scservo_python PRIVATE ${project})
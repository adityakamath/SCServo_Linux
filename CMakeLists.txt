
cmake_minimum_required(VERSION 3.5)
set(project "SCServo")
project(${project} LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O3")

# Set output directories for build artifacts
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

# Include SMS_STS, SCSCL, HLSCL, and base classes
# Phase 1 refactoring: Added utility headers for DRY compliance
set(hdrs src/SMS_STS.h src/SCSerial.h src/SCS.h src/SCSCL.h src/INST.h src/HLSCL.h
         src/ServoUtils.h src/SyncWriteBuffer.h src/ServoErrors.h src/SCServo.h)
set(srs src/SMS_STS.cpp src/SCSerial.cpp src/SCS.cpp src/SCSCL.cpp src/HLSCL.cpp)

add_library(${project} STATIC ${hdrs} ${srs})
target_include_directories(${project} PUBLIC ${CMAKE_SOURCE_DIR}/src)
set_target_properties(${project} PROPERTIES POSITION_INDEPENDENT_CODE ON)
#add_executable(${project} main.cpp ${hdrs} ${srs})

# --- Python bindings with nanobind ---
find_package(Python 3.8 REQUIRED COMPONENTS Interpreter Development.Module)
add_subdirectory(extern/nanobind)

nanobind_add_module(scservo_python src/bindings.cpp)
target_include_directories(scservo_python PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(scservo_python PRIVATE ${project})
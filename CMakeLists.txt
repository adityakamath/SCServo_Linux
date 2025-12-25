
cmake_minimum_required(VERSION 3.5)
set(project "SCServo")
project(${project} LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O3")

# Include SMS_STS, SCSCL, HLSCL, and base classes
# Phase 1 refactoring: Added utility headers for DRY compliance
set(hdrs SMS_STS.h SCSerial.h SCS.h SCSCL.h INST.h HLSCL.h
         ServoUtils.h SyncWriteBuffer.h ServoErrors.h)
set(srs SMS_STS.cpp SCSerial.cpp SCS.cpp SCSCL.cpp HLSCL.cpp)

add_library(${project} STATIC ${hdrs} ${srs})
set_target_properties(${project} PROPERTIES POSITION_INDEPENDENT_CODE ON)
#add_executable(${project} main.cpp ${hdrs} ${srs})

# --- Python bindings with nanobind ---
find_package(Python 3.8 REQUIRED COMPONENTS Interpreter Development.Module)
add_subdirectory(extern/nanobind)

nanobind_add_module(scservo_python src/bindings.cpp)
target_include_directories(scservo_python PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(scservo_python PRIVATE ${project})
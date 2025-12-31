cmake_minimum_required(VERSION 3.10)
set(project "SCServo")
project(${project} LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O3")

# Include SMS_STS, SCSCL, HLSCL, and base classes
# Phase 1 refactoring: Added utility headers for DRY compliance
set(hdrs include/SMS_STS.h include/SCSerial.h include/SCS.h include/SCSCL.h
         include/INST.h include/HLSCL.h include/ServoUtils.h
         include/SyncWriteBuffer.h include/ServoErrors.h include/SCServo.h)
set(srs src/SMS_STS.cpp src/SCSerial.cpp src/SCS.cpp src/SCSCL.cpp src/HLSCL.cpp)

add_library(${project} STATIC ${hdrs} ${srs})
target_include_directories(${project} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(${project} PROPERTIES POSITION_INDEPENDENT_CODE ON)
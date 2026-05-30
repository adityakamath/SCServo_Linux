cmake_minimum_required(VERSION 3.14)

project(SCServo
  VERSION 1.0.0
  LANGUAGES CXX
  DESCRIPTION "Feetech serial bus servo SDK for Linux")

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(SCSERVO_HEADERS
  include/INST.h
  include/SCS.h
  include/SCS0009.h
  include/SCSCL.h
  include/SCSerial.h
  include/SCServo.h
  include/SMS_STS.h
  include/HLSCL.h
  include/ServoErrors.h
  include/ServoUtils.h
  include/SyncWriteBuffer.h
)

set(SCSERVO_SOURCES
  src/SMS_STS.cpp
  src/SCSerial.cpp
  src/SCS.cpp
  src/SCS0009.cpp
  src/SCSCL.cpp
  src/HLSCL.cpp
)

add_library(SCServo STATIC ${SCSERVO_SOURCES} ${SCSERVO_HEADERS})

target_include_directories(SCServo PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/scservo>
)

target_compile_features(SCServo PRIVATE cxx_std_17)

set_target_properties(SCServo PROPERTIES
  POSITION_INDEPENDENT_CODE ON
)

install(TARGETS SCServo
  EXPORT SCServoTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/scservo
  FILES_MATCHING PATTERN "*.h"
)

install(EXPORT SCServoTargets
  FILE SCServoTargets.cmake
  NAMESPACE SCServo::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SCServo
)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/SCServoConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/SCServoConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SCServo
)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/SCServoConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMinorVersion
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/SCServoConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/SCServoConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SCServo
)

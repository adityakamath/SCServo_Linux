# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ubuntu/SCServo_Linux/examples/sandbox/MidpointCalib

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ubuntu/SCServo_Linux/examples/sandbox/MidpointCalib

# Include any dependencies generated for this target.
include CMakeFiles/Calibration.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/Calibration.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/Calibration.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Calibration.dir/flags.make

CMakeFiles/Calibration.dir/MidpointCalib.cpp.o: CMakeFiles/Calibration.dir/flags.make
CMakeFiles/Calibration.dir/MidpointCalib.cpp.o: MidpointCalib.cpp
CMakeFiles/Calibration.dir/MidpointCalib.cpp.o: CMakeFiles/Calibration.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/SCServo_Linux/examples/sandbox/MidpointCalib/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/Calibration.dir/MidpointCalib.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/Calibration.dir/MidpointCalib.cpp.o -MF CMakeFiles/Calibration.dir/MidpointCalib.cpp.o.d -o CMakeFiles/Calibration.dir/MidpointCalib.cpp.o -c /home/ubuntu/SCServo_Linux/examples/sandbox/MidpointCalib/MidpointCalib.cpp

CMakeFiles/Calibration.dir/MidpointCalib.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Calibration.dir/MidpointCalib.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/SCServo_Linux/examples/sandbox/MidpointCalib/MidpointCalib.cpp > CMakeFiles/Calibration.dir/MidpointCalib.cpp.i

CMakeFiles/Calibration.dir/MidpointCalib.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Calibration.dir/MidpointCalib.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/SCServo_Linux/examples/sandbox/MidpointCalib/MidpointCalib.cpp -o CMakeFiles/Calibration.dir/MidpointCalib.cpp.s

# Object files for target Calibration
Calibration_OBJECTS = \
"CMakeFiles/Calibration.dir/MidpointCalib.cpp.o"

# External object files for target Calibration
Calibration_EXTERNAL_OBJECTS =

Calibration: CMakeFiles/Calibration.dir/MidpointCalib.cpp.o
Calibration: CMakeFiles/Calibration.dir/build.make
Calibration: CMakeFiles/Calibration.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ubuntu/SCServo_Linux/examples/sandbox/MidpointCalib/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable Calibration"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Calibration.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/Calibration.dir/build: Calibration
.PHONY : CMakeFiles/Calibration.dir/build

CMakeFiles/Calibration.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Calibration.dir/cmake_clean.cmake
.PHONY : CMakeFiles/Calibration.dir/clean

CMakeFiles/Calibration.dir/depend:
	cd /home/ubuntu/SCServo_Linux/examples/sandbox/MidpointCalib && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ubuntu/SCServo_Linux/examples/sandbox/MidpointCalib /home/ubuntu/SCServo_Linux/examples/sandbox/MidpointCalib /home/ubuntu/SCServo_Linux/examples/sandbox/MidpointCalib /home/ubuntu/SCServo_Linux/examples/sandbox/MidpointCalib /home/ubuntu/SCServo_Linux/examples/sandbox/MidpointCalib/CMakeFiles/Calibration.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/Calibration.dir/depend


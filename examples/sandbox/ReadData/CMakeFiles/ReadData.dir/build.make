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
CMAKE_SOURCE_DIR = /home/ubuntu/SCServo_Linux/examples/sandbox/ReadData

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ubuntu/SCServo_Linux/examples/sandbox/ReadData

# Include any dependencies generated for this target.
include CMakeFiles/ReadData.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/ReadData.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/ReadData.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/ReadData.dir/flags.make

CMakeFiles/ReadData.dir/ReadData.cpp.o: CMakeFiles/ReadData.dir/flags.make
CMakeFiles/ReadData.dir/ReadData.cpp.o: ReadData.cpp
CMakeFiles/ReadData.dir/ReadData.cpp.o: CMakeFiles/ReadData.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/SCServo_Linux/examples/sandbox/ReadData/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/ReadData.dir/ReadData.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/ReadData.dir/ReadData.cpp.o -MF CMakeFiles/ReadData.dir/ReadData.cpp.o.d -o CMakeFiles/ReadData.dir/ReadData.cpp.o -c /home/ubuntu/SCServo_Linux/examples/sandbox/ReadData/ReadData.cpp

CMakeFiles/ReadData.dir/ReadData.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ReadData.dir/ReadData.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/SCServo_Linux/examples/sandbox/ReadData/ReadData.cpp > CMakeFiles/ReadData.dir/ReadData.cpp.i

CMakeFiles/ReadData.dir/ReadData.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ReadData.dir/ReadData.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/SCServo_Linux/examples/sandbox/ReadData/ReadData.cpp -o CMakeFiles/ReadData.dir/ReadData.cpp.s

# Object files for target ReadData
ReadData_OBJECTS = \
"CMakeFiles/ReadData.dir/ReadData.cpp.o"

# External object files for target ReadData
ReadData_EXTERNAL_OBJECTS =

ReadData: CMakeFiles/ReadData.dir/ReadData.cpp.o
ReadData: CMakeFiles/ReadData.dir/build.make
ReadData: CMakeFiles/ReadData.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ubuntu/SCServo_Linux/examples/sandbox/ReadData/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ReadData"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ReadData.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/ReadData.dir/build: ReadData
.PHONY : CMakeFiles/ReadData.dir/build

CMakeFiles/ReadData.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/ReadData.dir/cmake_clean.cmake
.PHONY : CMakeFiles/ReadData.dir/clean

CMakeFiles/ReadData.dir/depend:
	cd /home/ubuntu/SCServo_Linux/examples/sandbox/ReadData && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ubuntu/SCServo_Linux/examples/sandbox/ReadData /home/ubuntu/SCServo_Linux/examples/sandbox/ReadData /home/ubuntu/SCServo_Linux/examples/sandbox/ReadData /home/ubuntu/SCServo_Linux/examples/sandbox/ReadData /home/ubuntu/SCServo_Linux/examples/sandbox/ReadData/CMakeFiles/ReadData.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/ReadData.dir/depend

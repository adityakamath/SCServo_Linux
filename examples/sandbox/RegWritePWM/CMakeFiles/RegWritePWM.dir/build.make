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
CMAKE_SOURCE_DIR = /home/ubuntu/SCServo_Linux/examples/sandbox/RegWritePWM

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ubuntu/SCServo_Linux/examples/sandbox/RegWritePWM

# Include any dependencies generated for this target.
include CMakeFiles/RegWritePWM.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/RegWritePWM.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/RegWritePWM.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/RegWritePWM.dir/flags.make

CMakeFiles/RegWritePWM.dir/RegWritePWM.cpp.o: CMakeFiles/RegWritePWM.dir/flags.make
CMakeFiles/RegWritePWM.dir/RegWritePWM.cpp.o: RegWritePWM.cpp
CMakeFiles/RegWritePWM.dir/RegWritePWM.cpp.o: CMakeFiles/RegWritePWM.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/SCServo_Linux/examples/sandbox/RegWritePWM/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/RegWritePWM.dir/RegWritePWM.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/RegWritePWM.dir/RegWritePWM.cpp.o -MF CMakeFiles/RegWritePWM.dir/RegWritePWM.cpp.o.d -o CMakeFiles/RegWritePWM.dir/RegWritePWM.cpp.o -c /home/ubuntu/SCServo_Linux/examples/sandbox/RegWritePWM/RegWritePWM.cpp

CMakeFiles/RegWritePWM.dir/RegWritePWM.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/RegWritePWM.dir/RegWritePWM.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/SCServo_Linux/examples/sandbox/RegWritePWM/RegWritePWM.cpp > CMakeFiles/RegWritePWM.dir/RegWritePWM.cpp.i

CMakeFiles/RegWritePWM.dir/RegWritePWM.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/RegWritePWM.dir/RegWritePWM.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/SCServo_Linux/examples/sandbox/RegWritePWM/RegWritePWM.cpp -o CMakeFiles/RegWritePWM.dir/RegWritePWM.cpp.s

# Object files for target RegWritePWM
RegWritePWM_OBJECTS = \
"CMakeFiles/RegWritePWM.dir/RegWritePWM.cpp.o"

# External object files for target RegWritePWM
RegWritePWM_EXTERNAL_OBJECTS =

RegWritePWM: CMakeFiles/RegWritePWM.dir/RegWritePWM.cpp.o
RegWritePWM: CMakeFiles/RegWritePWM.dir/build.make
RegWritePWM: CMakeFiles/RegWritePWM.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ubuntu/SCServo_Linux/examples/sandbox/RegWritePWM/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable RegWritePWM"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/RegWritePWM.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/RegWritePWM.dir/build: RegWritePWM
.PHONY : CMakeFiles/RegWritePWM.dir/build

CMakeFiles/RegWritePWM.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/RegWritePWM.dir/cmake_clean.cmake
.PHONY : CMakeFiles/RegWritePWM.dir/clean

CMakeFiles/RegWritePWM.dir/depend:
	cd /home/ubuntu/SCServo_Linux/examples/sandbox/RegWritePWM && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ubuntu/SCServo_Linux/examples/sandbox/RegWritePWM /home/ubuntu/SCServo_Linux/examples/sandbox/RegWritePWM /home/ubuntu/SCServo_Linux/examples/sandbox/RegWritePWM /home/ubuntu/SCServo_Linux/examples/sandbox/RegWritePWM /home/ubuntu/SCServo_Linux/examples/sandbox/RegWritePWM/CMakeFiles/RegWritePWM.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/RegWritePWM.dir/depend


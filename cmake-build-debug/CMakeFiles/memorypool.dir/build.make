# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
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
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /mnt/e/CODE/C++/memorypool

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/e/CODE/C++/memorypool/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/memorypool.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/memorypool.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/memorypool.dir/flags.make

CMakeFiles/memorypool.dir/unitest.cpp.o: CMakeFiles/memorypool.dir/flags.make
CMakeFiles/memorypool.dir/unitest.cpp.o: ../unitest.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/e/CODE/C++/memorypool/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/memorypool.dir/unitest.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/memorypool.dir/unitest.cpp.o -c /mnt/e/CODE/C++/memorypool/unitest.cpp

CMakeFiles/memorypool.dir/unitest.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/memorypool.dir/unitest.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/e/CODE/C++/memorypool/unitest.cpp > CMakeFiles/memorypool.dir/unitest.cpp.i

CMakeFiles/memorypool.dir/unitest.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/memorypool.dir/unitest.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/e/CODE/C++/memorypool/unitest.cpp -o CMakeFiles/memorypool.dir/unitest.cpp.s

CMakeFiles/memorypool.dir/benchmark.cpp.o: CMakeFiles/memorypool.dir/flags.make
CMakeFiles/memorypool.dir/benchmark.cpp.o: ../benchmark.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/e/CODE/C++/memorypool/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/memorypool.dir/benchmark.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/memorypool.dir/benchmark.cpp.o -c /mnt/e/CODE/C++/memorypool/benchmark.cpp

CMakeFiles/memorypool.dir/benchmark.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/memorypool.dir/benchmark.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/e/CODE/C++/memorypool/benchmark.cpp > CMakeFiles/memorypool.dir/benchmark.cpp.i

CMakeFiles/memorypool.dir/benchmark.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/memorypool.dir/benchmark.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/e/CODE/C++/memorypool/benchmark.cpp -o CMakeFiles/memorypool.dir/benchmark.cpp.s

CMakeFiles/memorypool.dir/pagecache.cpp.o: CMakeFiles/memorypool.dir/flags.make
CMakeFiles/memorypool.dir/pagecache.cpp.o: ../pagecache.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/e/CODE/C++/memorypool/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/memorypool.dir/pagecache.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/memorypool.dir/pagecache.cpp.o -c /mnt/e/CODE/C++/memorypool/pagecache.cpp

CMakeFiles/memorypool.dir/pagecache.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/memorypool.dir/pagecache.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/e/CODE/C++/memorypool/pagecache.cpp > CMakeFiles/memorypool.dir/pagecache.cpp.i

CMakeFiles/memorypool.dir/pagecache.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/memorypool.dir/pagecache.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/e/CODE/C++/memorypool/pagecache.cpp -o CMakeFiles/memorypool.dir/pagecache.cpp.s

# Object files for target memorypool
memorypool_OBJECTS = \
"CMakeFiles/memorypool.dir/unitest.cpp.o" \
"CMakeFiles/memorypool.dir/benchmark.cpp.o" \
"CMakeFiles/memorypool.dir/pagecache.cpp.o"

# External object files for target memorypool
memorypool_EXTERNAL_OBJECTS =

memorypool: CMakeFiles/memorypool.dir/unitest.cpp.o
memorypool: CMakeFiles/memorypool.dir/benchmark.cpp.o
memorypool: CMakeFiles/memorypool.dir/pagecache.cpp.o
memorypool: CMakeFiles/memorypool.dir/build.make
memorypool: CMakeFiles/memorypool.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mnt/e/CODE/C++/memorypool/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX executable memorypool"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/memorypool.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/memorypool.dir/build: memorypool

.PHONY : CMakeFiles/memorypool.dir/build

CMakeFiles/memorypool.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/memorypool.dir/cmake_clean.cmake
.PHONY : CMakeFiles/memorypool.dir/clean

CMakeFiles/memorypool.dir/depend:
	cd /mnt/e/CODE/C++/memorypool/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/e/CODE/C++/memorypool /mnt/e/CODE/C++/memorypool /mnt/e/CODE/C++/memorypool/cmake-build-debug /mnt/e/CODE/C++/memorypool/cmake-build-debug /mnt/e/CODE/C++/memorypool/cmake-build-debug/CMakeFiles/memorypool.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/memorypool.dir/depend


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
CMAKE_COMMAND = /home/elanda/.local/share/JetBrains/apps/CLion/ch-0/201.7846.88/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /home/elanda/.local/share/JetBrains/apps/CLion/ch-0/201.7846.88/bin/cmake/linux/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/elanda/Documents/Development/Juce/Plugins/Cossin

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/elanda/Documents/Development/Juce/Plugins/Cossin/cmake-build-release

# Utility rule file for NightlyMemCheck.

# Include the progress variables for this target.
include externals/yaml-cpp/CMakeFiles/NightlyMemCheck.dir/progress.make

externals/yaml-cpp/CMakeFiles/NightlyMemCheck:
	cd /home/elanda/Documents/Development/Juce/Plugins/Cossin/cmake-build-release/externals/yaml-cpp && /home/elanda/.local/share/JetBrains/apps/CLion/ch-0/201.7846.88/bin/cmake/linux/bin/ctest -D NightlyMemCheck

NightlyMemCheck: externals/yaml-cpp/CMakeFiles/NightlyMemCheck
NightlyMemCheck: externals/yaml-cpp/CMakeFiles/NightlyMemCheck.dir/build.make

.PHONY : NightlyMemCheck

# Rule to build all files generated by this target.
externals/yaml-cpp/CMakeFiles/NightlyMemCheck.dir/build: NightlyMemCheck

.PHONY : externals/yaml-cpp/CMakeFiles/NightlyMemCheck.dir/build

externals/yaml-cpp/CMakeFiles/NightlyMemCheck.dir/clean:
	cd /home/elanda/Documents/Development/Juce/Plugins/Cossin/cmake-build-release/externals/yaml-cpp && $(CMAKE_COMMAND) -P CMakeFiles/NightlyMemCheck.dir/cmake_clean.cmake
.PHONY : externals/yaml-cpp/CMakeFiles/NightlyMemCheck.dir/clean

externals/yaml-cpp/CMakeFiles/NightlyMemCheck.dir/depend:
	cd /home/elanda/Documents/Development/Juce/Plugins/Cossin/cmake-build-release && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/elanda/Documents/Development/Juce/Plugins/Cossin /home/elanda/Documents/Development/Juce/Plugins/Cossin/externals/yaml-cpp /home/elanda/Documents/Development/Juce/Plugins/Cossin/cmake-build-release /home/elanda/Documents/Development/Juce/Plugins/Cossin/cmake-build-release/externals/yaml-cpp /home/elanda/Documents/Development/Juce/Plugins/Cossin/cmake-build-release/externals/yaml-cpp/CMakeFiles/NightlyMemCheck.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : externals/yaml-cpp/CMakeFiles/NightlyMemCheck.dir/depend


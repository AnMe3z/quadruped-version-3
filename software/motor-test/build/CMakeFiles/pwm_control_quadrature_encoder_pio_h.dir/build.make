# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.25

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
CMAKE_SOURCE_DIR = /home/andrey-shefa/Documents/repos/quadruped-version-3/software/motor-test

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/andrey-shefa/Documents/repos/quadruped-version-3/software/motor-test/build

# Utility rule file for pwm_control_quadrature_encoder_pio_h.

# Include any custom commands dependencies for this target.
include CMakeFiles/pwm_control_quadrature_encoder_pio_h.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/pwm_control_quadrature_encoder_pio_h.dir/progress.make

CMakeFiles/pwm_control_quadrature_encoder_pio_h: quadrature_encoder.pio.h

quadrature_encoder.pio.h: /home/andrey-shefa/Documents/repos/quadruped-version-3/software/motor-test/quadrature_encoder.pio
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/andrey-shefa/Documents/repos/quadruped-version-3/software/motor-test/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating quadrature_encoder.pio.h"
	pioasm/pioasm -o c-sdk /home/andrey-shefa/Documents/repos/quadruped-version-3/software/motor-test/quadrature_encoder.pio /home/andrey-shefa/Documents/repos/quadruped-version-3/software/motor-test/build/quadrature_encoder.pio.h

pwm_control_quadrature_encoder_pio_h: CMakeFiles/pwm_control_quadrature_encoder_pio_h
pwm_control_quadrature_encoder_pio_h: quadrature_encoder.pio.h
pwm_control_quadrature_encoder_pio_h: CMakeFiles/pwm_control_quadrature_encoder_pio_h.dir/build.make
.PHONY : pwm_control_quadrature_encoder_pio_h

# Rule to build all files generated by this target.
CMakeFiles/pwm_control_quadrature_encoder_pio_h.dir/build: pwm_control_quadrature_encoder_pio_h
.PHONY : CMakeFiles/pwm_control_quadrature_encoder_pio_h.dir/build

CMakeFiles/pwm_control_quadrature_encoder_pio_h.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/pwm_control_quadrature_encoder_pio_h.dir/cmake_clean.cmake
.PHONY : CMakeFiles/pwm_control_quadrature_encoder_pio_h.dir/clean

CMakeFiles/pwm_control_quadrature_encoder_pio_h.dir/depend:
	cd /home/andrey-shefa/Documents/repos/quadruped-version-3/software/motor-test/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/andrey-shefa/Documents/repos/quadruped-version-3/software/motor-test /home/andrey-shefa/Documents/repos/quadruped-version-3/software/motor-test /home/andrey-shefa/Documents/repos/quadruped-version-3/software/motor-test/build /home/andrey-shefa/Documents/repos/quadruped-version-3/software/motor-test/build /home/andrey-shefa/Documents/repos/quadruped-version-3/software/motor-test/build/CMakeFiles/pwm_control_quadrature_encoder_pio_h.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/pwm_control_quadrature_encoder_pio_h.dir/depend


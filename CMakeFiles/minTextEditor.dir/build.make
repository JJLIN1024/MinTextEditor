# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.26

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
CMAKE_COMMAND = /opt/homebrew/Cellar/cmake/3.26.4/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/Cellar/cmake/3.26.4/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/jimmylin/MinTextEditor

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/jimmylin/MinTextEditor

# Include any dependencies generated for this target.
include CMakeFiles/minTextEditor.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/minTextEditor.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/minTextEditor.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/minTextEditor.dir/flags.make

CMakeFiles/minTextEditor.dir/src/main.c.o: CMakeFiles/minTextEditor.dir/flags.make
CMakeFiles/minTextEditor.dir/src/main.c.o: src/main.c
CMakeFiles/minTextEditor.dir/src/main.c.o: CMakeFiles/minTextEditor.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/jimmylin/MinTextEditor/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/minTextEditor.dir/src/main.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/minTextEditor.dir/src/main.c.o -MF CMakeFiles/minTextEditor.dir/src/main.c.o.d -o CMakeFiles/minTextEditor.dir/src/main.c.o -c /Users/jimmylin/MinTextEditor/src/main.c

CMakeFiles/minTextEditor.dir/src/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/minTextEditor.dir/src/main.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/jimmylin/MinTextEditor/src/main.c > CMakeFiles/minTextEditor.dir/src/main.c.i

CMakeFiles/minTextEditor.dir/src/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/minTextEditor.dir/src/main.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/jimmylin/MinTextEditor/src/main.c -o CMakeFiles/minTextEditor.dir/src/main.c.s

CMakeFiles/minTextEditor.dir/src/editor.c.o: CMakeFiles/minTextEditor.dir/flags.make
CMakeFiles/minTextEditor.dir/src/editor.c.o: src/editor.c
CMakeFiles/minTextEditor.dir/src/editor.c.o: CMakeFiles/minTextEditor.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/jimmylin/MinTextEditor/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/minTextEditor.dir/src/editor.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/minTextEditor.dir/src/editor.c.o -MF CMakeFiles/minTextEditor.dir/src/editor.c.o.d -o CMakeFiles/minTextEditor.dir/src/editor.c.o -c /Users/jimmylin/MinTextEditor/src/editor.c

CMakeFiles/minTextEditor.dir/src/editor.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/minTextEditor.dir/src/editor.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/jimmylin/MinTextEditor/src/editor.c > CMakeFiles/minTextEditor.dir/src/editor.c.i

CMakeFiles/minTextEditor.dir/src/editor.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/minTextEditor.dir/src/editor.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/jimmylin/MinTextEditor/src/editor.c -o CMakeFiles/minTextEditor.dir/src/editor.c.s

# Object files for target minTextEditor
minTextEditor_OBJECTS = \
"CMakeFiles/minTextEditor.dir/src/main.c.o" \
"CMakeFiles/minTextEditor.dir/src/editor.c.o"

# External object files for target minTextEditor
minTextEditor_EXTERNAL_OBJECTS =

minTextEditor: CMakeFiles/minTextEditor.dir/src/main.c.o
minTextEditor: CMakeFiles/minTextEditor.dir/src/editor.c.o
minTextEditor: CMakeFiles/minTextEditor.dir/build.make
minTextEditor: CMakeFiles/minTextEditor.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/jimmylin/MinTextEditor/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable minTextEditor"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/minTextEditor.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/minTextEditor.dir/build: minTextEditor
.PHONY : CMakeFiles/minTextEditor.dir/build

CMakeFiles/minTextEditor.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/minTextEditor.dir/cmake_clean.cmake
.PHONY : CMakeFiles/minTextEditor.dir/clean

CMakeFiles/minTextEditor.dir/depend:
	cd /Users/jimmylin/MinTextEditor && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/jimmylin/MinTextEditor /Users/jimmylin/MinTextEditor /Users/jimmylin/MinTextEditor /Users/jimmylin/MinTextEditor /Users/jimmylin/MinTextEditor/CMakeFiles/minTextEditor.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/minTextEditor.dir/depend

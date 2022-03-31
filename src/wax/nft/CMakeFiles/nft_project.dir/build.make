# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

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
CMAKE_SOURCE_DIR = /code/src/wax/nft

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /code/src/wax/nft

# Utility rule file for nft_project.

# Include the progress variables for this target.
include CMakeFiles/nft_project.dir/progress.make

CMakeFiles/nft_project: CMakeFiles/nft_project-complete


CMakeFiles/nft_project-complete: nft_project-prefix/src/nft_project-stamp/nft_project-install
CMakeFiles/nft_project-complete: nft_project-prefix/src/nft_project-stamp/nft_project-mkdir
CMakeFiles/nft_project-complete: nft_project-prefix/src/nft_project-stamp/nft_project-download
CMakeFiles/nft_project-complete: nft_project-prefix/src/nft_project-stamp/nft_project-update
CMakeFiles/nft_project-complete: nft_project-prefix/src/nft_project-stamp/nft_project-patch
CMakeFiles/nft_project-complete: nft_project-prefix/src/nft_project-stamp/nft_project-configure
CMakeFiles/nft_project-complete: nft_project-prefix/src/nft_project-stamp/nft_project-build
CMakeFiles/nft_project-complete: nft_project-prefix/src/nft_project-stamp/nft_project-install
CMakeFiles/nft_project-complete: nft_project-prefix/src/nft_project-stamp/nft_project-test
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/code/src/wax/nft/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Completed 'nft_project'"
	/usr/bin/cmake -E make_directory /code/src/wax/nft/CMakeFiles
	/usr/bin/cmake -E touch /code/src/wax/nft/CMakeFiles/nft_project-complete
	/usr/bin/cmake -E touch /code/src/wax/nft/nft_project-prefix/src/nft_project-stamp/nft_project-done

nft_project-prefix/src/nft_project-stamp/nft_project-install: nft_project-prefix/src/nft_project-stamp/nft_project-build
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/code/src/wax/nft/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "No install step for 'nft_project'"
	cd /code/src/wax/nft/nft && /usr/bin/cmake -E echo_append
	cd /code/src/wax/nft/nft && /usr/bin/cmake -E touch /code/src/wax/nft/nft_project-prefix/src/nft_project-stamp/nft_project-install

nft_project-prefix/src/nft_project-stamp/nft_project-mkdir:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/code/src/wax/nft/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Creating directories for 'nft_project'"
	/usr/bin/cmake -E make_directory /code/src/wax/nft/src
	/usr/bin/cmake -E make_directory /code/src/wax/nft/nft
	/usr/bin/cmake -E make_directory /code/src/wax/nft/nft_project-prefix
	/usr/bin/cmake -E make_directory /code/src/wax/nft/nft_project-prefix/tmp
	/usr/bin/cmake -E make_directory /code/src/wax/nft/nft_project-prefix/src/nft_project-stamp
	/usr/bin/cmake -E make_directory /code/src/wax/nft/nft_project-prefix/src
	/usr/bin/cmake -E touch /code/src/wax/nft/nft_project-prefix/src/nft_project-stamp/nft_project-mkdir

nft_project-prefix/src/nft_project-stamp/nft_project-download: nft_project-prefix/src/nft_project-stamp/nft_project-mkdir
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/code/src/wax/nft/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "No download step for 'nft_project'"
	/usr/bin/cmake -E echo_append
	/usr/bin/cmake -E touch /code/src/wax/nft/nft_project-prefix/src/nft_project-stamp/nft_project-download

nft_project-prefix/src/nft_project-stamp/nft_project-update: nft_project-prefix/src/nft_project-stamp/nft_project-download
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/code/src/wax/nft/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "No update step for 'nft_project'"
	cd /code/src/wax/nft/src && /usr/bin/cmake -E echo_append
	cd /code/src/wax/nft/src && /usr/bin/cmake -E touch /code/src/wax/nft/nft_project-prefix/src/nft_project-stamp/nft_project-update

nft_project-prefix/src/nft_project-stamp/nft_project-patch: nft_project-prefix/src/nft_project-stamp/nft_project-download
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/code/src/wax/nft/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "No patch step for 'nft_project'"
	cd /code/src/wax/nft/src && /usr/bin/cmake -E echo_append
	cd /code/src/wax/nft/src && /usr/bin/cmake -E touch /code/src/wax/nft/nft_project-prefix/src/nft_project-stamp/nft_project-patch

nft_project-prefix/src/nft_project-stamp/nft_project-configure: nft_project-prefix/tmp/nft_project-cfgcmd.txt
nft_project-prefix/src/nft_project-stamp/nft_project-configure: nft_project-prefix/src/nft_project-stamp/nft_project-update
nft_project-prefix/src/nft_project-stamp/nft_project-configure: nft_project-prefix/src/nft_project-stamp/nft_project-patch
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/code/src/wax/nft/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Performing configure step for 'nft_project'"
	cd /code/src/wax/nft/nft && /usr/bin/cmake -DCMAKE_TOOLCHAIN_FILE=/usr/local/eosio.cdt/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake "-GUnix Makefiles" /code/src/wax/nft/src
	cd /code/src/wax/nft/nft && /usr/bin/cmake -E touch /code/src/wax/nft/nft_project-prefix/src/nft_project-stamp/nft_project-configure

nft_project-prefix/src/nft_project-stamp/nft_project-build: nft_project-prefix/src/nft_project-stamp/nft_project-configure
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/code/src/wax/nft/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Performing build step for 'nft_project'"
	cd /code/src/wax/nft/nft && $(MAKE)

nft_project-prefix/src/nft_project-stamp/nft_project-test: nft_project-prefix/src/nft_project-stamp/nft_project-install
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/code/src/wax/nft/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "No test step for 'nft_project'"
	cd /code/src/wax/nft/nft && /usr/bin/cmake -E echo_append
	cd /code/src/wax/nft/nft && /usr/bin/cmake -E touch /code/src/wax/nft/nft_project-prefix/src/nft_project-stamp/nft_project-test

nft_project: CMakeFiles/nft_project
nft_project: CMakeFiles/nft_project-complete
nft_project: nft_project-prefix/src/nft_project-stamp/nft_project-install
nft_project: nft_project-prefix/src/nft_project-stamp/nft_project-mkdir
nft_project: nft_project-prefix/src/nft_project-stamp/nft_project-download
nft_project: nft_project-prefix/src/nft_project-stamp/nft_project-update
nft_project: nft_project-prefix/src/nft_project-stamp/nft_project-patch
nft_project: nft_project-prefix/src/nft_project-stamp/nft_project-configure
nft_project: nft_project-prefix/src/nft_project-stamp/nft_project-build
nft_project: nft_project-prefix/src/nft_project-stamp/nft_project-test
nft_project: CMakeFiles/nft_project.dir/build.make

.PHONY : nft_project

# Rule to build all files generated by this target.
CMakeFiles/nft_project.dir/build: nft_project

.PHONY : CMakeFiles/nft_project.dir/build

CMakeFiles/nft_project.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/nft_project.dir/cmake_clean.cmake
.PHONY : CMakeFiles/nft_project.dir/clean

CMakeFiles/nft_project.dir/depend:
	cd /code/src/wax/nft && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /code/src/wax/nft /code/src/wax/nft /code/src/wax/nft /code/src/wax/nft /code/src/wax/nft/CMakeFiles/nft_project.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/nft_project.dir/depend


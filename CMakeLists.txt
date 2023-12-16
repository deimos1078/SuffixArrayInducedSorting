cmake_minimum_required(VERSION 3.10)
project(SuffixArrayProject)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Library for shared code
add_library(suffix_array_lib STATIC src/SuffixArray.cpp)
add_library(brute_force_lib tests/BruteForce.cpp)

# Tests
enable_testing()

# Basic tests
add_executable(BasicTests tests/BasicTests.cpp)
target_link_libraries(BasicTests suffix_array_lib)

# Time tests (not run automatically)
add_executable(TimeTests tests/TimeTests.cpp)
target_link_libraries(TimeTests suffix_array_lib)
target_link_libraries(TimeTests brute_force_lib)

# Assuming your data files are in a directory named 'data' at the project root
set(DATA_FILES_DIR ${CMAKE_SOURCE_DIR}/data)
set(DATA_FILES_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/data)

# Create 'data' directory in the build directory
file(MAKE_DIRECTORY ${DATA_FILES_BUILD_DIR})

# Define file lists
set(patternFiles "patterns10.txt" "patterns100.txt" "patterns1000.txt" "patterns10000.txt" "patterns100000.txt")
set(stringFiles "string1000000.txt" "string2500000.txt" "string5000000.txt" "string10000000.txt")

# Combine lists
set(allFiles ${stringFiles} ${patternFiles})

# Copy data files to 'data' directory in the build directory
foreach(FILE IN LISTS allFiles)
    configure_file(${DATA_FILES_DIR}/${FILE} ${DATA_FILES_BUILD_DIR}/${FILE} COPYONLY)
endforeach()

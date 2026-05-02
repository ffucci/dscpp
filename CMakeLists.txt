cmake_minimum_required(VERSION 3.20)

# set the project name
project(cpp_learn)

set(CMAKE_CXX_COMPILER c++)
# Set the C++ standard
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_RPATH "/opt/gcc/gcc-16.1.0/lib64")
set(CMAKE_INSTALL_RPATH "/opt/gcc/gcc-16.1.0/lib64")
# Add GoogleTest
find_package(GTest REQUIRED CONFIG)
include_directories(${GTEST_INCLUDE_DIRS})

# Enable testing needs to be before everything
enable_testing()

# add the executable
add_subdirectory(apps)
add_subdirectory(lib)
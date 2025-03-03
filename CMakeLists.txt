cmake_minimum_required(VERSION 3.20)

# set the project name
project(Template)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# Add GoogleTest
find_package(GTest REQUIRED CONFIG)
include_directories(${GTEST_INCLUDE_DIRS})

# Enable testing needs to be before everything
enable_testing()

# add the executable
add_subdirectory(apps)
add_subdirectory(lib)
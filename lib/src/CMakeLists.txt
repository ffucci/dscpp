find_package(spdlog REQUIRED CONFIG)

# Find all source files in the src directory
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/lib/src/*.cpp")
# Print the source files
message(STATUS "Source files in ${PROJECT_SOURCE_DIR}: ${SOURCES}")

# Add the library
add_library(cpplearn STATIC ${SOURCES})


target_compile_options(cpplearn PRIVATE
    -mavx2
    -Wall        # Enable all warnings
    -Wextra      # Enable extra warnings
    -Wpedantic   # Enforce strict standard compliance
    -O3          # Optimize for speed
)


message(STATUS "Conan: Target declared 'cpplearn'" ${PROJECT_SOURCE_DIR}/lib/include)

# Optionally, specify include directories
target_include_directories(cpplearn PUBLIC
    ${PROJECT_SOURCE_DIR}/lib/include
)

target_link_libraries(cpplearn PUBLIC spdlog::spdlog)

find_package(spdlog REQUIRED CONFIG)

# include_directories(${CMAKE_SOURCE_DIR}/lib/include)

add_executable(btree_run main.cpp)

target_compile_options(btree_run PRIVATE
    -mavx2
    -Wall        # Enable all warnings
    -Wextra      # Enable extra warnings
    -Wpedantic   # Enforce strict standard compliance
    -O3          # Optimize for speed
)
target_link_libraries(btree_run PUBLIC cpplearn spdlog::spdlog)
find_package(spdlog REQUIRED CONFIG)

# include_directories(${CMAKE_SOURCE_DIR}/lib/include)

add_executable(example_probe example_probe.cpp)

target_compile_options(example_probe PRIVATE
    -mavx2
    -Wall        # Enable all warnings
    -Wextra      # Enable extra warnings
    -Wpedantic   # Enforce strict standard compliance
    -O3          # Optimize for speed
)
target_link_libraries(example_probe PUBLIC cpplearn)

add_executable(rbuf_run main.cpp)

target_compile_options(rbuf_run PRIVATE
    -march=native
    -Wall        # Enable all warnings
    -Wextra      # Enable extra warnings
    -Wpedantic   # Enforce strict standard compliance
    -O3          # Optimize for speed
)
target_link_libraries(rbuf_run PUBLIC cpplearn spdlog::spdlog)

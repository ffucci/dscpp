add_executable(memory_run main.cpp)

target_compile_options(memory_run PRIVATE
    -mavx2
    -Wall        # Enable all warnings
    -Wextra      # Enable extra warnings
    -Wpedantic   # Enforce strict standard compliance
    -O3          # Optimize for speed
)
target_link_libraries(memory_run PUBLIC cpplearn)

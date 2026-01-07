add_executable(variant_run main.cpp)

target_compile_options(variant_run PRIVATE
    -mavx2
    -Wall        # Enable all warnings
    -Wextra      # Enable extra warnings
    -Wpedantic   # Enforce strict standard compliance
    -O3          # Optimize for speed
)
target_link_libraries(variant_run PUBLIC cpplearn)
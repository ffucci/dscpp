add_executable(simd_run main.cpp)

target_compile_options(simd_run PRIVATE
    -mavx2
    -Wall        # Enable all warnings
    -Wextra      # Enable extra warnings
    -Wpedantic   # Enforce strict standard compliance
    -O3          # Optimize for speed
)
target_link_libraries(simd_run PUBLIC ds_exp)
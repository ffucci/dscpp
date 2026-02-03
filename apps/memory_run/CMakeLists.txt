add_executable(memory_run main.cpp)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(memory_run PRIVATE
            -DDEBUG
    )

    target_compile_options(memory_run PRIVATE
            -Wall        # Enable all warnings
            -Wextra      # Enable extra warnings
            -Wpedantic   # Enforce strict standard compliance
            -g
            -O0
    )
else ()
    target_compile_options(memory_run PRIVATE
            -mavx2
            -Wall        # Enable all warnings
            -Wextra      # Enable extra warnings
            -Wpedantic   # Enforce strict standard compliance
            -O3          # Optimize for speed
    )
endif ()

target_link_libraries(memory_run PUBLIC cpplearn)

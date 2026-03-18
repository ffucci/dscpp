add_executable(hashtable_bench
    hashtable_benchmark.cpp
)

target_include_directories(hashtable_bench PRIVATE
    ${PROJECT_SOURCE_DIR}/lib/include
    ${PROJECT_SOURCE_DIR}/lib/bench
)

target_link_libraries(hashtable_bench PRIVATE cpplearn)

target_compile_options(hashtable_bench PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -O3
)

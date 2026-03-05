find_package(spdlog REQUIRED CONFIG)
find_package(Threads REQUIRED)

add_executable(dbuf_run main.cpp)

target_compile_options(dbuf_run PRIVATE
    -march=native
    -Wall
    -Wextra
    -Wpedantic
    -O3
)

target_link_libraries(dbuf_run PUBLIC cpplearn spdlog::spdlog Threads::Threads)

# Add GoogleTest
find_package(GTest REQUIRED)

add_executable(tests_all
        main.cpp
        tests_exception_guard.cpp
        tests_mvector.cpp)

target_link_libraries(tests_all PUBLIC cpplearn GTest::GTest GTest::Main)
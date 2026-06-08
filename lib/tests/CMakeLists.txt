# Add GoogleTest
find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(tests_all
        main.cpp
        test_weak_ptr.cpp
        tests_exception_guard.cpp
        tests_mvector.cpp
        tests_variant.cpp
        tests_shared_ptr.cpp)

target_link_libraries(tests_all PUBLIC cpplearn GTest::GTest GTest::Main)
gtest_discover_tests(tests_all)

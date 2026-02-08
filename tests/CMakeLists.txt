# Tests for refinery library

add_executable(test_refine test_refine.cpp)
target_link_libraries(test_refine PRIVATE refinery::refinery GTest::gtest_main)
target_compile_options(test_refine PRIVATE -Wall -Wextra -Werror)

# Discover tests automatically
include(GoogleTest)
gtest_discover_tests(test_refine
    PROPERTIES TIMEOUT 60
)

add_executable(test_refined_container test_refined_container.cpp)
target_link_libraries(test_refined_container PRIVATE refinery::refinery GTest::gtest_main)
target_compile_options(test_refined_container PRIVATE -Wall -Wextra -Werror)

gtest_discover_tests(test_refined_container
    PROPERTIES TIMEOUT 60
)

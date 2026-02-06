# Tests for rcpp library

add_executable(test_refine test_refine.cpp)
target_link_libraries(test_refine PRIVATE rcpp::rcpp)

# Add test
add_test(NAME test_refine COMMAND test_refine)

# Set test properties
set_tests_properties(test_refine PROPERTIES
    TIMEOUT 60
)

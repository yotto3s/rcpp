# examples/CMakeLists.txt — Zero-overhead assembly comparison examples

set(ZERO_OVERHEAD_EXAMPLES
    01_value_passthrough
    02_arithmetic
    03_safe_sqrt
    04_parameter_passing
    05_comparison
    06_multiply
    07_safe_divide
    08_chain
)

set(EXAMPLE_TARGETS "")

foreach(example IN LISTS ZERO_OVERHEAD_EXAMPLES)
    set(target "zero_overhead_${example}")
    add_executable(${target} "zero_overhead/${example}.cpp")
    target_link_libraries(${target} PRIVATE rcpp::rcpp)
    target_compile_options(${target} PRIVATE -O2)
    list(APPEND EXAMPLE_TARGETS ${target})
endforeach()

# Umbrella target to build all examples
add_custom_target(examples-all DEPENDS ${EXAMPLE_TARGETS})

# Assembly comparison target
add_custom_target(asm-compare
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/compare_asm.sh
            ${CMAKE_CURRENT_BINARY_DIR}
            ${ZERO_OVERHEAD_EXAMPLES}
    DEPENDS ${EXAMPLE_TARGETS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
    COMMENT "Comparing refined vs plain assembly output"
    VERBATIM
)

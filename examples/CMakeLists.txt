# examples/CMakeLists.txt — Assembly comparison examples

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

set(RUNTIME_OVERHEAD_EXAMPLES
    01_runtime_check_positive
    02_runtime_check_nonzero
    03_runtime_check_inrange
    04_checked_addition
    05_checked_subtraction
)

set(ZERO_OVERHEAD_TARGETS "")
set(RUNTIME_OVERHEAD_TARGETS "")

foreach(example IN LISTS ZERO_OVERHEAD_EXAMPLES)
    set(target "zero_overhead_${example}")
    add_executable(${target} "zero_overhead/${example}.cpp")
    target_link_libraries(${target} PRIVATE refinery::refinery)
    target_compile_options(${target} PRIVATE -O2 -Wall -Wextra -Werror -Wno-unused-but-set-variable)
    list(APPEND ZERO_OVERHEAD_TARGETS ${target})
endforeach()

foreach(example IN LISTS RUNTIME_OVERHEAD_EXAMPLES)
    set(target "runtime_overhead_${example}")
    add_executable(${target} "runtime_overhead/${example}.cpp")
    target_link_libraries(${target} PRIVATE refinery::refinery)
    target_compile_options(${target} PRIVATE -O2 -Wall -Wextra -Werror -Wno-unused-but-set-variable)
    list(APPEND RUNTIME_OVERHEAD_TARGETS ${target})
endforeach()

add_subdirectory(containers)

# Umbrella target to build all examples
add_custom_target(examples-all DEPENDS ${ZERO_OVERHEAD_TARGETS} ${RUNTIME_OVERHEAD_TARGETS} container-examples)

# Assembly comparison targets
add_custom_target(asm-compare
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/compare_asm.sh
            ${CMAKE_CURRENT_BINARY_DIR}
            zero_overhead_
            ${ZERO_OVERHEAD_EXAMPLES}
    DEPENDS ${ZERO_OVERHEAD_TARGETS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
    COMMENT "Comparing refined vs plain assembly output (zero overhead)"
    VERBATIM
)

add_custom_target(asm-compare-runtime
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/compare_asm.sh
            ${CMAKE_CURRENT_BINARY_DIR}
            runtime_overhead_
            ${RUNTIME_OVERHEAD_EXAMPLES}
    DEPENDS ${RUNTIME_OVERHEAD_TARGETS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
    COMMENT "Comparing refined vs plain assembly output (runtime overhead)"
    VERBATIM
)

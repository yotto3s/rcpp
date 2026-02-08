# examples/containers/CMakeLists.txt — RefinedContainer showcase examples

set(CONTAINER_EXAMPLES
    01_construction
    02_predicate_gated_access
    03_mutations
    04_static_indexing
    05_range_insertion
    06_freeze_guard
)

set(CONTAINER_TARGETS "")

foreach(example IN LISTS CONTAINER_EXAMPLES)
    set(target "container_${example}")
    add_executable(${target} "${example}.cpp")
    target_link_libraries(${target} PRIVATE refinery::refinery)
    # GCC produces false-positive -Warray-bounds warnings in std::vector's
    # push_back/emplace_back inlining at -O2 (tracks initial allocation size
    # through reallocation paths)
    target_compile_options(${target} PRIVATE -O2 -Wall -Wextra -Werror -Wno-unused-but-set-variable -Wno-array-bounds)
    list(APPEND CONTAINER_TARGETS ${target})
endforeach()

add_custom_target(container-examples DEPENDS ${CONTAINER_TARGETS})

// 01_construction.cpp — RefinedContainer construction modes
//
// Demonstrates four ways to construct a RefinedContainer:
//   1. consteval      — compile-time verified (no tag, like Refined<T,P>)
//   2. runtime_check  — validates size predicate at runtime
//   3. assume_valid   — unchecked, for trusted contexts
//   4. SizeRefined    — convenience alias with explicit size range

#include <refinery/refinery.hpp>

#include <array>
#include <cassert>
#include <iostream>
#include <vector>

using namespace refinery;

int main() {
    // 1. consteval — compile-time verified construction (no tag)
    //    Like Refined<T,P>, the default constructor is consteval.
    //    A size violation here is a compile error.
    constexpr auto compile_time = SizeRefined<std::array<int, 3>, 3, 3>(
        std::array{10, 20, 30});
    std::cout << "consteval:     size = " << compile_time.size() << "\n";
    static_assert(compile_time.size() == 3);

    // 2. runtime_check — construct NonEmptyContainer from a populated vector
    //    Throws refinement_error if the vector is empty.
    auto vec1 = std::vector{10, 20, 30};
    auto non_empty = NonEmptyContainer<std::vector<int>>(std::move(vec1),
                                                         runtime_check);
    std::cout << "runtime_check: size = " << non_empty.size() << "\n";
    assert(non_empty.size() == 3);

    // 3. assume_valid — unchecked construction for trusted contexts
    //    Caller guarantees the predicate holds. No runtime cost.
    auto vec2 = std::vector{1, 2};
    auto trusted = NonEmptyContainer<std::vector<int>>(std::move(vec2),
                                                       assume_valid);
    std::cout << "assume_valid:  size = " << trusted.size() << "\n";
    assert(trusted.size() == 2);

    // 4. SizeRefined — exact size range [3, 5]
    //    Container must have between 3 and 5 elements.
    auto vec3 = std::vector{1, 2, 3, 4};
    auto sized = SizeRefined<std::vector<int>, 3, 5>(std::move(vec3),
                                                      runtime_check);
    std::cout << "SizeRefined:   size = " << sized.size() << "\n";
    assert(sized.size() == 4);

    // COMPILE ERROR: front() is not available when container might be empty
    // SizeInterval<0> has lo=0, so the non-empty constraint is not satisfied.
    // auto maybe_empty = SizeRefined<std::vector<int>, 0, 10>(
    //     std::vector<int>{}, runtime_check);
    // maybe_empty.front();

    std::cout << "All construction modes work.\n";
    return 0;
}

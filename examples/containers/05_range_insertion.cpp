// 05_range_insertion.cpp — Batch append with predicate propagation
//
// append() from std::array shifts bounds by exactly N.
// append() from RefinedContainer shifts bounds by the source's lower bound.

#include <refinery/refinery.hpp>

#include <array>
#include <cassert>
#include <iostream>
#include <vector>

using namespace refinery;

int main() {
    // Start with SizeInterval<2, 4> — size is in [2, 4]
    auto rc = SizeRefined<std::vector<int>, 2, 4>(
        std::vector{10, 20}, runtime_check);

    // append(std::array<int, 3>) — bounds shift by exactly 3
    // [2, 4] -> [5, 7]
    auto rc2 = std::move(rc).append(std::array{30, 40, 50});
    static_assert(decltype(rc2)::size_predicate.lo == 5);
    static_assert(decltype(rc2)::size_predicate.hi == 7);
    std::cout << "After append(array<3>): size = " << rc2.size()
              << ", bounds = [" << decltype(rc2)::size_predicate.lo
              << ", " << decltype(rc2)::size_predicate.hi << "]\n";
    assert(rc2.size() == 5);

    // append(RefinedContainer) — bounds shift by source's lower bound
    auto source = SizeRefined<std::vector<int>, 2, 3>(
        std::vector{60, 70}, runtime_check);
    // [5, 7] + lo(source)=2 -> [7, ...]
    auto rc3 = std::move(rc2).append(std::move(source));
    static_assert(decltype(rc3)::size_predicate.lo == 7);
    std::cout << "After append(RefinedContainer): size = " << rc3.size()
              << ", lo = " << decltype(rc3)::size_predicate.lo << "\n";
    assert(rc3.size() == 7);

    // Verify all elements
    std::cout << "elements:";
    for (int x : rc3) {
        std::cout << " " << x;
    }
    std::cout << "\n";

    std::cout << "Range insertion works.\n";
    return 0;
}

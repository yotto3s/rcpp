// 04_static_indexing.cpp — Compile-time bounds-checked indexing
//
// RefinedContainer accepts Refined<size_t, Interval<Lo, Hi>> indices.
// The index compiles only when the index upper bound is provably less
// than the container's size lower bound.

#include <refinery/refinery.hpp>

#include <cassert>
#include <iostream>
#include <vector>

using namespace refinery;

int main() {
    // Container with SizeInterval<5> — guaranteed at least 5 elements
    auto rc = SizeRefined<std::vector<int>, 5>(
        std::vector{10, 20, 30, 40, 50}, runtime_check);

    // Index with Interval<0, 4>: upper bound 4 < container lower bound 5
    // This is statically safe — no runtime bounds check needed.
    auto idx0 = Refined<std::size_t, Interval<0uz, 4uz>{}>(0uz, assume_valid);
    auto idx3 = Refined<std::size_t, Interval<0uz, 4uz>{}>(3uz, assume_valid);

    std::cout << "rc[0] = " << rc[idx0] << "\n";
    std::cout << "rc[3] = " << rc[idx3] << "\n";
    assert(rc[idx0] == 10);
    assert(rc[idx3] == 40);

    // A single-value index also works
    auto idx_last = Refined<std::size_t, Interval<4uz, 4uz>{}>(4uz, assume_valid);
    std::cout << "rc[4] = " << rc[idx_last] << "\n";
    assert(rc[idx_last] == 50);

    // COMPILE ERROR: index Interval<0, 5> on SizeInterval<5>
    // Index upper bound (5) >= container lower bound (5) — access could be
    // out-of-bounds.
    // auto bad_idx = Refined<std::size_t, Interval<0uz, 5uz>{}>(
    //     3uz, assume_valid);
    // rc[bad_idx];

    std::cout << "Static indexing works.\n";
    return 0;
}

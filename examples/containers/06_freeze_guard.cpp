// 06_freeze_guard.cpp — Runtime indexing with compile-time brand safety
//
// freeze() captures the container's size and produces a (SizeGuard,
// FrozenContainer) pair. The guard produces GuardedIndex values that
// are type-branded — they can only be used with the matching frozen
// container.

#include <refinery/refinery.hpp>

#include <cassert>
#include <iostream>
#include <vector>

using namespace refinery;

int main() {
    // Build a container and freeze it
    auto rc = NonEmptyContainer<std::vector<int>>(
        std::vector{10, 20, 30, 40, 50}, runtime_check);

    auto [guard, frozen] = std::move(rc).freeze();

    // Use guard to produce a checked index
    auto maybe_idx = guard.check(2);
    assert(maybe_idx.has_value());
    std::cout << "frozen[2] = " << frozen[*maybe_idx] << "\n";
    assert(frozen[*maybe_idx] == 30);

    // Out-of-bounds check returns std::nullopt
    auto bad_idx = guard.check(99);
    assert(!bad_idx.has_value());
    std::cout << "check(99) = nullopt (out of bounds)\n";

    // Iterate through all valid indices
    std::cout << "all elements via guard:";
    for (std::size_t i = 0; i < guard.size(); ++i) {
        auto idx = guard.check(i);
        assert(idx.has_value());
        std::cout << " " << frozen[*idx];
    }
    std::cout << "\n";

    // COMPILE ERROR: brand mismatch
    // GuardedIndex from one freeze() call cannot be used with a different
    // FrozenContainer — the lambda-based Tag types are unique per call site.
    // auto rc2 = NonEmptyContainer<std::vector<int>>(
    //     std::vector{100, 200}, runtime_check);
    // auto [guard2, frozen2] = std::move(rc2).freeze();
    // auto idx2 = guard2.check(0);
    // frozen[*idx2]; // ERROR: Tag mismatch

    std::cout << "Freeze/guard works.\n";
    return 0;
}

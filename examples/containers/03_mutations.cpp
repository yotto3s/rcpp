// 03_mutations.cpp — Move-based type-changing mutations
//
// Mutating operations consume *this and return a new RefinedContainer
// with updated size bounds. The old variable is moved-from.

#include <refinery/refinery.hpp>

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

using namespace refinery;

int main() {
    // Start with SizeInterval<2, 5> — size is in [2, 5]
    auto rc = SizeRefined<std::vector<int>, 2, 5>(
        std::vector{10, 20}, runtime_check);

    // push_back: [2,5] -> [3,6]
    auto rc2 = std::move(rc).push_back(30);
    // rc is now moved-from — do not use it
    static_assert(decltype(rc2)::size_predicate.lo == 3);
    static_assert(decltype(rc2)::size_predicate.hi == 6);
    std::cout << "After push_back(30):  size = " << rc2.size() << "\n";
    assert(rc2.size() == 3);

    // pop_back: [3,6] -> [2,5]
    auto rc3 = std::move(rc2).pop_back();
    static_assert(decltype(rc3)::size_predicate.lo == 2);
    static_assert(decltype(rc3)::size_predicate.hi == 5);
    std::cout << "After pop_back():     size = " << rc3.size() << "\n";
    assert(rc3.size() == 2);

    // emplace_back with constructor arguments: [2,5] -> [3,6]
    auto strs = SizeRefined<std::vector<std::string>, 1, 3>(
        std::vector<std::string>{"hello"}, runtime_check);
    auto strs2 = std::move(strs).emplace_back(5, 'x'); // "xxxxx"
    std::cout << "After emplace_back(): " << strs2.back() << "\n";
    assert(strs2.back() == "xxxxx");

    // Chained mutations: push_back three times
    auto chained = SizeRefined<std::vector<int>, 0, 0>(
        std::vector<int>{}, runtime_check);
    auto result = std::move(chained)
        .push_back(1)
        .push_back(2)
        .push_back(3);
    // [0,0] -> [1,1] -> [2,2] -> [3,3]
    static_assert(decltype(result)::size_predicate.lo == 3);
    static_assert(decltype(result)::size_predicate.hi == 3);
    std::cout << "After chained push_back: size = " << result.size() << "\n";
    assert(result.size() == 3);

    // COMPILE ERROR: pop_back() on SizeInterval<0>
    // Can't pop from a potentially empty container.
    // auto empty = SizeRefined<std::vector<int>, 0, 5>(
    //     std::vector{1, 2}, runtime_check);
    // std::move(empty).pop_back();

    std::cout << "All mutations work.\n";
    return 0;
}

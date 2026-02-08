// 02_predicate_gated_access.cpp — Compile-time gated element access
//
// front() and back() are only available when the size predicate
// guarantees the container is non-empty (lo >= 1).

#include <refinery/refinery.hpp>

#include <cassert>
#include <iostream>
#include <vector>

using namespace refinery;

int main() {
    // NonEmptyContainer has SizeInterval<1> — lo=1, so front()/back() compile.
    auto ne = NonEmptyContainer<std::vector<int>>(
        std::vector{10, 20, 30}, runtime_check);

    // Predicate-gated: these only compile because lo >= 1
    std::cout << "front() = " << ne.front() << "\n";
    std::cout << "back()  = " << ne.back() << "\n";
    assert(ne.front() == 10);
    assert(ne.back() == 30);

    // Range-for iteration via begin()/end() — always available
    std::cout << "elements:";
    for (int x : ne) {
        std::cout << " " << x;
    }
    std::cout << "\n";

    // size() and data() — always available
    std::cout << "size()  = " << ne.size() << "\n";
    std::cout << "data()  = " << ne.data() << " (pointer)\n";
    assert(ne.size() == 3);
    assert(ne.data() != nullptr);

    // COMPILE ERROR: front() on SizeRefined<..., 0, 10>
    // lo=0 means the container might be empty, so front() is constrained away.
    // auto maybe_empty = SizeRefined<std::vector<int>, 0, 10>(
    //     std::vector{1, 2, 3}, runtime_check);
    // maybe_empty.front();

    std::cout << "Predicate-gated access works.\n";
    return 0;
}

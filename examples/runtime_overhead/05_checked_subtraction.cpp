// 05_checked_subtraction.cpp — Proves interval-based integer subtraction with
// overflow checking matches hand-written checked subtraction equivalent
//
// PositiveInt - PositiveInt has trivially-wide bounds, so the result degrades
// to plain int. The runtime cost is just checked_sub. The plain version does
// the same overflow check manually.
//
// Expected: refined_sub_positive and plain_sub_positive produce identical assembly.

#include <refinery/refinery.hpp>

using namespace refinery;

__attribute__((noinline)) int
refined_sub_positive(PositiveInt a, PositiveInt b) {
    return a - b;
}

__attribute__((noinline)) int
plain_sub_positive(int a, int b) {
    return detail::checked_sub(a, b);
}

int main() {
    auto a = PositiveInt(30, assume_valid);
    auto b = PositiveInt(10, assume_valid);

    volatile int sink;
    sink = refined_sub_positive(a, b);
    sink = plain_sub_positive(30, 10);
    return 0;
}

// 04_checked_addition.cpp — Proves interval-based integer addition with overflow
// checking matches hand-written checked addition equivalent
//
// For integers, interval operators use checked_add which throws on overflow.
// The plain version does the same overflow check manually.
//
// Expected: refined_add_positive and plain_add_positive produce identical assembly.

#include <refinery/refinery.hpp>

using namespace refinery;

__attribute__((noinline)) int
refined_add_positive(PositiveI32 a, PositiveI32 b) {
    return (a + b).get();
}

__attribute__((noinline)) int
plain_add_positive(int a, int b) {
    return detail::checked_add(a, b);
}

int main() {
    auto a = PositiveI32(10, assume_valid);
    auto b = PositiveI32(20, assume_valid);

    volatile int sink;
    sink = refined_add_positive(a, b);
    sink = plain_add_positive(10, 20);
    return 0;
}

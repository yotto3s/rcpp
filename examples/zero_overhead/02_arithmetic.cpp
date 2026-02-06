// 02_arithmetic.cpp — Proves Positive + Positive == int + int
//
// The preserves<Positive, plus> trait lets operator+ use assume_valid,
// so the addition should compile to a single `add` with no branches.

#include <rcpp/refine.hpp>

using namespace refine;

__attribute__((noinline))
int refined_add(Refined<int, Positive> a, Refined<int, Positive> b) {
    return (a + b).get();
}

__attribute__((noinline))
int plain_add(int a, int b) {
    return a + b;
}

int main() {
    auto a = Refined<int, Positive>(10, assume_valid);
    auto b = Refined<int, Positive>(20, assume_valid);
    volatile int sink;
    sink = refined_add(a, b);
    sink = plain_add(10, 20);
    return 0;
}

// 06_multiply.cpp — Proves Positive * Positive == int * int
//
// The preserves<Positive, multiplies> trait lets operator* use assume_valid,
// so the multiplication should compile to a single `imul` with no branches.

#include <rcpp/refine.hpp>

using namespace refine;

__attribute__((noinline))
int refined_mul(Refined<int, Positive> a, Refined<int, Positive> b) {
    return (a * b).get();
}

__attribute__((noinline))
int plain_mul(int a, int b) {
    return a * b;
}

int main() {
    auto a = Refined<int, Positive>(6, assume_valid);
    auto b = Refined<int, Positive>(7, assume_valid);
    volatile int sink;
    sink = refined_mul(a, b);
    sink = plain_mul(6, 7);
    return 0;
}

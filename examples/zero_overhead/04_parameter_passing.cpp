// 04_parameter_passing.cpp — Proves Refined<int> passes in registers like int
//
// Single-param and multi-param variants. Refined<int, Pred> has the same
// layout as int, so it should use the same calling convention.

#include <rcpp/refine.hpp>

using namespace refine;

__attribute__((noinline))
int refined_sum3(Refined<int, Positive> a,
                 Refined<int, Positive> b,
                 Refined<int, Positive> c) {
    return a.get() + b.get() + c.get();
}

__attribute__((noinline))
int plain_sum3(int a, int b, int c) {
    return a + b + c;
}

int main() {
    auto a = Refined<int, Positive>(1, assume_valid);
    auto b = Refined<int, Positive>(2, assume_valid);
    auto c = Refined<int, Positive>(3, assume_valid);
    volatile int sink;
    sink = refined_sum3(a, b, c);
    sink = plain_sum3(1, 2, 3);
    return 0;
}

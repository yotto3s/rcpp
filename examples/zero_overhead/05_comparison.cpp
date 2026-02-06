// 05_comparison.cpp — Proves operator< and operator== via spaceship == plain
//
// Refined's comparison operators delegate to the underlying value's
// three-way comparison, which should produce identical instructions.

#include <rcpp/refine.hpp>

using namespace refine;

__attribute__((noinline))
bool refined_less(Refined<int, Positive> a, Refined<int, Positive> b) {
    return a < b;
}

__attribute__((noinline))
bool plain_less(int a, int b) {
    return a < b;
}

__attribute__((noinline))
bool refined_equal(Refined<int, Positive> a, Refined<int, Positive> b) {
    return a == b;
}

__attribute__((noinline))
bool plain_equal(int a, int b) {
    return a == b;
}

int main() {
    auto a = Refined<int, Positive>(5, assume_valid);
    auto b = Refined<int, Positive>(10, assume_valid);
    volatile bool sink;
    sink = refined_less(a, b);
    sink = plain_less(5, 10);
    sink = refined_equal(a, b);
    sink = plain_equal(5, 10);
    return 0;
}

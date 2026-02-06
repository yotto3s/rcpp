// 07_safe_divide.cpp — Proves safe_divide / safe_reciprocal == plain division
//
// safe_divide takes a NonZero denominator and performs plain division.
// safe_reciprocal computes 1.0 / x for NonZero x.

#include <rcpp/refine.hpp>

using namespace refine;

__attribute__((noinline))
double refined_divide(double num, Refined<double, NonZero> den) {
    return safe_divide(num, den);
}

__attribute__((noinline))
double plain_divide(double num, double den) {
    return num / den;
}

__attribute__((noinline))
double refined_reciprocal(Refined<double, NonZero> x) {
    return safe_reciprocal(x);
}

__attribute__((noinline))
double plain_reciprocal(double x) {
    return 1.0 / x;
}

int main() {
    auto d = Refined<double, NonZero>(3.0, assume_valid);
    volatile double sink;
    sink = refined_divide(10.0, d);
    sink = plain_divide(10.0, 3.0);
    sink = refined_reciprocal(d);
    sink = plain_reciprocal(3.0);
    return 0;
}

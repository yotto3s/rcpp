// 06_multiply.cpp — Proves Positive<double> * Positive<double> == double * double
//
// The preserves<Positive, multiplies, double> trait lets operator* use
// assume_valid for floating-point types, so multiplication compiles to a
// single `mulsd` with no branches.
//
// For integers, overflow can violate Positive, so operator* returns
// std::optional and includes a runtime check (branch).

#include <rcpp/refined.hpp>

using namespace refined;

// --- Floating-point: zero overhead ---

__attribute__((noinline))
double refined_mul(Refined<double, Positive> a, Refined<double, Positive> b) {
    return (a * b).get();
}

__attribute__((noinline))
double plain_mul(double a, double b) {
    return a * b;
}

int main() {
    auto a = Refined<double, Positive>(6.0, assume_valid);
    auto b = Refined<double, Positive>(7.0, assume_valid);
    volatile double sink;
    sink = refined_mul(a, b);
    sink = plain_mul(6.0, 7.0);
    return 0;
}

// 02_arithmetic.cpp — Proves Positive<double> + Positive<double> == double + double
//
// The preserves<Positive, plus, double> trait lets operator+ use assume_valid
// for floating-point types, so the addition compiles to a single `addsd`
// with no branches.
//
// For integers, overflow can violate Positive, so operator+ returns
// std::optional and includes a runtime check (branch).

#include <rcpp/refined.hpp>

using namespace refined;

// --- Floating-point: zero overhead ---

__attribute__((noinline))
double refined_add(Refined<double, Positive> a, Refined<double, Positive> b) {
    return (a + b).get();
}

__attribute__((noinline))
double plain_add(double a, double b) {
    return a + b;
}

int main() {
    auto a = Refined<double, Positive>(10.0, assume_valid);
    auto b = Refined<double, Positive>(20.0, assume_valid);
    volatile double sink;
    sink = refined_add(a, b);
    sink = plain_add(10.0, 20.0);
    return 0;
}

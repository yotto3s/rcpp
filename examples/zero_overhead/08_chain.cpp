// 08_chain.cpp — Proves multi-op chains match plain equivalents
//
// Chain: square(x) + square(y) -> safe_sqrt
// This exercises square (returns NonNegative), addition (preserves NonNegative),
// and safe_sqrt (preserves NonNegative).

#include <rcpp/refine.hpp>
#include <cmath>

using namespace refine;

__attribute__((noinline))
double refined_hypot(Refined<double, NonNegative> x,
                     Refined<double, NonNegative> y) {
    auto x2 = square(x);   // Refined<double, NonNegative>
    auto y2 = square(y);   // Refined<double, NonNegative>
    auto sum = x2 + y2;    // Refined<double, NonNegative> (preserves)
    return safe_sqrt(sum).get();
}

__attribute__((noinline))
double plain_hypot(double x, double y) {
    double x2 = x * x;
    double y2 = y * y;
    double sum = x2 + y2;
    return std::sqrt(sum);
}

int main() {
    auto x = Refined<double, NonNegative>(3.0, assume_valid);
    auto y = Refined<double, NonNegative>(4.0, assume_valid);
    volatile double sink;
    sink = refined_hypot(x, y);
    sink = plain_hypot(3.0, 4.0);
    return 0;
}

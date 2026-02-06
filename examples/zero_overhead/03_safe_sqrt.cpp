// 03_safe_sqrt.cpp — Proves safe_sqrt(NonNegative) == std::sqrt()
//
// safe_sqrt accepts Refined<double, NonNegative> and returns the same type,
// using assume_valid internally. Should compile to a single sqrtsd.

#include <rcpp/refined.hpp>
#include <cmath>

using namespace refined;

__attribute__((noinline))
double refined_sqrt(Refined<double, NonNegative> x) {
    return safe_sqrt(x).get();
}

__attribute__((noinline))
double plain_sqrt(double x) {
    return std::sqrt(x);
}

int main() {
    auto x = Refined<double, NonNegative>(9.0, assume_valid);
    volatile double sink;
    sink = refined_sqrt(x);
    sink = plain_sqrt(9.0);
    return 0;
}

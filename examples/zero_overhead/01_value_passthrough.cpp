// 01_value_passthrough.cpp — Proves .get() accessor is free
//
// Expected: refined_passthrough and plain_passthrough produce identical assembly.

#include <rcpp/refined.hpp>

using namespace refined;

__attribute__((noinline))
int refined_passthrough(Refined<int, Positive> x) {
    return x.get();
}

__attribute__((noinline))
int plain_passthrough(int x) {
    return x;
}

int main() {
    auto a = Refined<int, Positive>(42, assume_valid);
    volatile int sink;
    sink = refined_passthrough(a);
    sink = plain_passthrough(42);
    return 0;
}

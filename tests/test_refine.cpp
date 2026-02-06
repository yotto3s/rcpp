// test_refine.cpp - Test suite for C++26 Refinement Types Library
// Compile with: g++ -std=c++26 -freflection test_refine.cpp -o test_refine

#include <rcpp/refine.hpp>
#include <iostream>
#include <cassert>
#include <cmath>
#include <limits>
#include <numbers>

using namespace refine;

// Test compile-time construction
consteval int test_compile_time_construction() {
    // In consteval context, variables are implicitly available for constant evaluation
    PositiveInt p1{42};
    PositiveInt p2{1};

    NonZeroInt nz{-5};
    NonNegativeInt nn{0};

    Percentage pct{50};

    // Test value access (assertions, not static_assert, in consteval)
    if (p1.get() != 42) throw "p1 should be 42";
    if (*p2 != 1) throw "p2 should be 1";
    if (nz.get() != -5) throw "nz should be -5";
    if (nn.get() != 0) throw "nn should be 0";
    if (pct.get() != 50) throw "pct should be 50";

    return p1.get() + p2.get();
}

// Test that compile-time construction fails for invalid values
// Uncomment to see the compile error with reflection diagnostics:
// consteval int test_compile_time_failure() {
//     PositiveInt p{-1};  // Should fail: -1 is not positive
//     return p.get();
// }

// Test runtime construction
void test_runtime_construction() {
    // Runtime checked construction
    try {
        PositiveInt p{42, runtime_check};
        assert(p.get() == 42);
        std::cout << "Runtime construction of PositiveInt{42} succeeded\n";
    } catch (const refinement_error& e) {
        std::cout << "Unexpected error: " << e.what() << "\n";
        assert(false);
    }

    // Runtime construction with invalid value
    try {
        PositiveInt p{-1, runtime_check};
        std::cout << "Should have thrown!\n";
        assert(false);
    } catch (const refinement_error& e) {
        std::cout << "Expected error caught: " << e.what() << "\n";
    }
}

// Test try_refine
void test_try_refine() {
    // Valid value
    auto maybe_positive = try_refine<PositiveInt>(42);
    assert(maybe_positive.has_value());
    assert(maybe_positive->get() == 42);
    std::cout << "try_refine<PositiveInt>(42) = " << maybe_positive->get() << "\n";

    // Invalid value
    auto maybe_negative = try_refine<PositiveInt>(-1);
    assert(!maybe_negative.has_value());
    std::cout << "try_refine<PositiveInt>(-1) = nullopt\n";

    // Test with explicit predicate
    auto maybe_even = try_refine<Even>(4);
    assert(maybe_even.has_value());
    assert(maybe_even->get() == 4);

    auto maybe_odd_even = try_refine<Even>(3);
    assert(!maybe_odd_even.has_value());
}

// Test predicates
void test_predicates() {
    // Basic predicates
    static_assert(Positive(5));
    static_assert(!Positive(-5));
    static_assert(!Positive(0));

    static_assert(NonZero(5));
    static_assert(NonZero(-5));
    static_assert(!NonZero(0));

    static_assert(NonNegative(0));
    static_assert(NonNegative(5));
    static_assert(!NonNegative(-5));

    // Range predicates
    constexpr auto in_0_100 = InRange(0, 100);
    static_assert(in_0_100(0));
    static_assert(in_0_100(50));
    static_assert(in_0_100(100));
    static_assert(!in_0_100(-1));
    static_assert(!in_0_100(101));

    // Comparison predicates
    constexpr auto gt_10 = GreaterThan(10);
    static_assert(gt_10(11));
    static_assert(!gt_10(10));
    static_assert(!gt_10(5));

    // Divisibility
    static_assert(Even(4));
    static_assert(!Even(3));
    static_assert(Odd(3));
    static_assert(!Odd(4));
    static_assert(DivisibleBy(3)(9));
    static_assert(!DivisibleBy(3)(10));

    std::cout << "All predicate tests passed\n";
}

// Test predicate composition
void test_composition() {
    // All composition
    constexpr auto positive_and_even = All<Positive, Even>;
    static_assert(positive_and_even(4));
    static_assert(!positive_and_even(-4));  // Not positive
    static_assert(!positive_and_even(3));   // Not even

    // Any composition
    constexpr auto positive_or_even = Any<Positive, Even>;
    static_assert(positive_or_even(3));   // Positive but not even
    static_assert(positive_or_even(-4));  // Even but not positive
    static_assert(!positive_or_even(-3)); // Neither

    // Not composition
    constexpr auto not_positive = Not<Positive>;
    static_assert(not_positive(-5));
    static_assert(not_positive(0));
    static_assert(!not_positive(5));

    // If composition (implication)
    constexpr auto even_implies_positive = If<Even, Positive>;
    static_assert(even_implies_positive(4));   // Even and positive
    static_assert(!even_implies_positive(-4)); // Even but not positive
    static_assert(even_implies_positive(3));   // Not even, so implication holds

    std::cout << "All composition tests passed\n";
}

// Test safe operations
void test_operations() {
    // Safe division
    constexpr NonZeroInt denom{2};
    constexpr int result = safe_divide(10, denom);
    static_assert(result == 5);

    // Absolute value produces NonNegative
    constexpr auto abs_neg = refine::abs(-5);
    static_assert(abs_neg.get() == 5);
    static_assert(NonNegative(abs_neg.get()));

    // Square produces NonNegative
    constexpr auto sq = refine::square(-3);
    static_assert(sq.get() == 9);
    static_assert(NonNegative(sq.get()));

    // Min/max preserve refinement
    constexpr PositiveInt a{5};
    constexpr PositiveInt b{3};
    constexpr auto min_ab = refined_min(a, b);
    static_assert(min_ab.get() == 3);

    std::cout << "All operation tests passed\n";
}

// Test type aliases
void test_type_aliases() {
    // Percentage
    constexpr Percentage pct{75};
    static_assert(pct.get() == 75);

    // Probability
    constexpr Probability prob{0.5};
    static_assert(prob.get() == 0.5);

    // ByteValue
    constexpr ByteValue byte{255};
    static_assert(byte.get() == 255);

    // PortNumber
    constexpr PortNumber port{8080};
    static_assert(port.get() == 8080);

    std::cout << "All type alias tests passed\n";
}

// Test implicit conversion
void test_conversion() {
    constexpr PositiveInt p{42};

    // Implicit conversion to underlying type
    constexpr int i = p;
    static_assert(i == 42);

    // Works with functions expecting the underlying type
    auto square_int = [](int x) { return x * x; };
    int squared = square_int(p);
    assert(squared == 1764);

    std::cout << "All conversion tests passed\n";
}

// Test formatting
void test_formatting() {
    PositiveInt p{42, runtime_check};
    std::string formatted = std::format("Value: {}", p);
    assert(formatted == "Value: 42");
    std::cout << "Formatted: " << formatted << "\n";
}

// Example: Type-safe array index
template<std::size_t N>
using BoundedIndex = Refined<std::size_t, InHalfOpenRange(std::size_t{0}, N)>;

template<typename T, std::size_t N>
constexpr const T& safe_at(const T (&arr)[N], BoundedIndex<N> index) {
    return arr[index.get()];
}

void test_safe_array_access() {
    constexpr int arr[] = {10, 20, 30, 40, 50};

    // Create a refined index
    constexpr BoundedIndex<5> idx{2};
    constexpr int value = arr[idx.get()];  // Safe access
    static_assert(value == 30);

    std::cout << "Safe array access test passed\n";
}

// Example: Function that requires positive input
template<typename T>
constexpr T sqrt_positive(Refined<T, Positive> x)
    requires std::floating_point<T>
{
    // x is guaranteed to be positive, so sqrt is safe
    // (simplified Newton-Raphson implementation)
    T guess = x.get() / 2;
    for (int i = 0; i < 10; ++i) {
        guess = (guess + x.get() / guess) / 2;
    }
    return guess;
}

void test_sqrt_example() {
    PositiveDouble pd{4.0, runtime_check};
    double result = sqrt_positive(pd);
    std::cout << "sqrt(4.0) = " << result << "\n";
    assert(result > 1.9 && result < 2.1);  // Approximately 2
}

// Test comparison operations
void test_comparisons() {
    constexpr PositiveInt a{5};
    constexpr PositiveInt b{3};
    constexpr PositiveInt c{5};

    static_assert(a == c);
    static_assert(!(a == b));
    static_assert(a > b);
    static_assert(b < a);
    static_assert(a >= c);
    static_assert(b <= a);

    // Comparison with raw value
    static_assert(a == 5);
    static_assert(a > 3);

    std::cout << "All comparison tests passed\n";
}

// Test is_valid static method
void test_is_valid() {
    static_assert(PositiveInt::is_valid(5));
    static_assert(!PositiveInt::is_valid(-5));
    static_assert(!PositiveInt::is_valid(0));

    static_assert(NonZeroInt::is_valid(5));
    static_assert(NonZeroInt::is_valid(-5));
    static_assert(!NonZeroInt::is_valid(0));

    std::cout << "All is_valid tests passed\n";
}

// Test float predicates
void test_float_predicates() {
    // NotNaN
    static_assert(NotNaN(1.0));
    static_assert(NotNaN(0.0));
    static_assert(NotNaN(-1.0));
    assert(!NotNaN(std::numeric_limits<double>::quiet_NaN()));

    // IsNaN
    assert(IsNaN(std::numeric_limits<double>::quiet_NaN()));
    assert(IsNaN(std::numeric_limits<float>::quiet_NaN()));
    static_assert(!IsNaN(1.0));
    static_assert(!IsNaN(0.0));

    // Finite
    static_assert(Finite(1.0));
    static_assert(Finite(0.0));
    static_assert(Finite(-1.0));
    assert(!Finite(std::numeric_limits<double>::infinity()));
    assert(!Finite(-std::numeric_limits<double>::infinity()));
    assert(!Finite(std::numeric_limits<double>::quiet_NaN()));

    // IsInf
    assert(IsInf(std::numeric_limits<double>::infinity()));
    assert(IsInf(-std::numeric_limits<double>::infinity()));
    static_assert(!IsInf(1.0));
    static_assert(!IsInf(0.0));
    assert(!IsInf(std::numeric_limits<double>::quiet_NaN()));

    // IsNormal
    static_assert(IsNormal(1.0));
    static_assert(IsNormal(-1.0));
    static_assert(!IsNormal(0.0));

    // ApproxEqual
    constexpr auto near_zero = ApproxEqual(0.0, 0.001);
    static_assert(near_zero(0.0));
    static_assert(near_zero(0.0005));
    static_assert(near_zero(-0.0005));
    static_assert(!near_zero(0.01));
    static_assert(!near_zero(-0.01));

    constexpr auto near_pi = ApproxEqual(3.14159, 0.01);
    static_assert(near_pi(3.14));
    static_assert(!near_pi(3.0));

    std::cout << "All float predicate tests passed\n";
}

// Test compile-time construction of float refined types
consteval double test_float_compile_time() {
    PositiveDouble pd{3.14};
    FiniteDouble fd{2.718};
    NormalizedDouble nd{0.5};
    UnitDouble ud{0.75};

    if (pd.get() != 3.14) throw "pd should be 3.14";
    if (fd.get() != 2.718) throw "fd should be 2.718";
    if (nd.get() != 0.5) throw "nd should be 0.5";
    if (ud.get() != 0.75) throw "ud should be 0.75";

    return pd.get();
}

// Test runtime construction with float values
void test_float_runtime_construction() {
    // Valid constructions
    FiniteDouble fd{1.5, runtime_check};
    assert(fd.get() == 1.5);

    NormalizedDouble nd{-0.5, runtime_check};
    assert(nd.get() == -0.5);

    UnitDouble ud{0.5, runtime_check};
    assert(ud.get() == 0.5);

    // Invalid: NaN for Finite
    try {
        FiniteDouble bad{std::numeric_limits<double>::quiet_NaN(), runtime_check};
        assert(false && "Should have thrown");
    } catch (const refinement_error& e) {
        std::cout << "Expected Finite NaN error: " << e.what() << "\n";
    }

    // Invalid: infinity for Finite
    try {
        FiniteDouble bad{std::numeric_limits<double>::infinity(), runtime_check};
        assert(false && "Should have thrown");
    } catch (const refinement_error& e) {
        std::cout << "Expected Finite Inf error: " << e.what() << "\n";
    }

    // Invalid: 2.0 for Normalized (must be in [-1, 1])
    try {
        NormalizedDouble bad{2.0, runtime_check};
        assert(false && "Should have thrown");
    } catch (const refinement_error& e) {
        std::cout << "Expected Normalized error: " << e.what() << "\n";
    }

    // Invalid: -0.1 for UnitDouble (must be in [0, 1])
    try {
        UnitDouble bad{-0.1, runtime_check};
        assert(false && "Should have thrown");
    } catch (const refinement_error& e) {
        std::cout << "Expected Unit error: " << e.what() << "\n";
    }

    std::cout << "All float runtime construction tests passed\n";
}

// Test safe math operations
void test_float_operations() {
    // safe_sqrt with NonNegative
    NonNegativeDouble nn{4.0, runtime_check};
    auto sqrt_nn = refine::safe_sqrt(nn);
    assert(std::abs(sqrt_nn.get() - 2.0) < 1e-10);
    assert(NonNegative(sqrt_nn.get()));

    // safe_sqrt with Positive
    PositiveDouble pd{9.0, runtime_check};
    auto sqrt_pd = refine::safe_sqrt(pd);
    assert(std::abs(sqrt_pd.get() - 3.0) < 1e-10);
    assert(Positive(sqrt_pd.get()));

    // safe_sqrt of zero
    NonNegativeDouble zero{0.0, runtime_check};
    auto sqrt_zero = refine::safe_sqrt(zero);
    assert(sqrt_zero.get() == 0.0);

    // safe_log
    PositiveDouble e_val{std::numbers::e, runtime_check};
    double log_e = refine::safe_log(e_val);
    assert(std::abs(log_e - 1.0) < 1e-10);

    PositiveDouble one{1.0, runtime_check};
    double log_one = refine::safe_log(one);
    assert(std::abs(log_one) < 1e-10);

    // safe_asin
    NormalizedDouble half{0.5, runtime_check};
    double asin_half = refine::safe_asin(half);
    assert(std::abs(asin_half - std::asin(0.5)) < 1e-10);

    // safe_acos
    double acos_half = refine::safe_acos(half);
    assert(std::abs(acos_half - std::acos(0.5)) < 1e-10);

    // safe_reciprocal
    NonZeroDouble nz{4.0, runtime_check};
    double recip = refine::safe_reciprocal(nz);
    assert(std::abs(recip - 0.25) < 1e-10);

    NonZeroDouble neg_nz{-2.0, runtime_check};
    double recip_neg = refine::safe_reciprocal(neg_nz);
    assert(std::abs(recip_neg - (-0.5)) < 1e-10);

    std::cout << "All float operation tests passed\n";
}

// Test float edge cases
void test_float_edge_cases() {
    // Negative zero is still zero, so NonNegative and Finite should accept it
    double neg_zero = -0.0;
    assert(NonNegative(neg_zero));  // -0.0 >= 0.0 is true
    assert(Finite(neg_zero));

    // Largest finite value
    constexpr double max_val = std::numeric_limits<double>::max();
    static_assert(Finite(max_val));
    static_assert(Positive(max_val));

    // Smallest normal value
    constexpr double min_normal = std::numeric_limits<double>::min();
    static_assert(Positive(min_normal));
    static_assert(IsNormal(min_normal));

    // Denormalized value
    constexpr double denorm = std::numeric_limits<double>::denorm_min();
    static_assert(Positive(denorm));
    static_assert(Finite(denorm));
    static_assert(!IsNormal(denorm));  // Denormals are not normal

    // Float vs double
    static_assert(Finite(1.0f));
    static_assert(Finite(1.0));
    assert(!Finite(std::numeric_limits<float>::infinity()));
    assert(!Finite(std::numeric_limits<double>::infinity()));

    // Float type aliases work
    FiniteFloat ff{1.5f, runtime_check};
    assert(ff.get() == 1.5f);

    NormalizedFloat nf{-0.5f, runtime_check};
    assert(nf.get() == -0.5f);

    std::cout << "All float edge case tests passed\n";
}

int main() {
    std::cout << "=== C++26 Refinement Types Test Suite ===\n\n";

    // Compile-time test (runs at compile time)
    constexpr int ct_result = test_compile_time_construction();
    static_assert(ct_result == 43);
    std::cout << "Compile-time construction: passed\n";

    // Compile-time float test
    constexpr double ct_float_result = test_float_compile_time();
    static_assert(ct_float_result == 3.14);
    std::cout << "Compile-time float construction: passed\n";

    // Runtime tests
    test_runtime_construction();
    test_try_refine();
    test_predicates();
    test_composition();
    test_operations();
    test_type_aliases();
    test_conversion();
    test_formatting();
    test_safe_array_access();
    test_sqrt_example();
    test_comparisons();
    test_is_valid();
    test_float_predicates();
    test_float_runtime_construction();
    test_float_operations();
    test_float_edge_cases();

    std::cout << "\n=== All tests passed! ===\n";
    return 0;
}

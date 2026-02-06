// test_refine.cpp - Test suite for C++26 Refinement Types Library

#include <rcpp/refine.hpp>
#include <gtest/gtest.h>
#include <cmath>
#include <limits>
#include <numbers>

using namespace refine;

// ---- Helper templates used by tests ----

template<std::size_t N>
using BoundedIndex = Refined<std::size_t, InHalfOpenRange(std::size_t{0}, N)>;

template<typename T, std::size_t N>
constexpr const T& safe_at(const T (&arr)[N], BoundedIndex<N> index) {
    return arr[index.get()];
}

template<typename T>
constexpr T sqrt_positive(Refined<T, Positive> x)
    requires std::floating_point<T>
{
    T guess = x.get() / 2;
    for (int i = 0; i < 10; ++i) {
        guess = (guess + x.get() / guess) / 2;
    }
    return guess;
}

// ---- Consteval test functions ----

consteval int test_compile_time_construction() {
    PositiveInt p1{42};
    PositiveInt p2{1};

    NonZeroInt nz{-5};
    NonNegativeInt nn{0};

    Percentage pct{50};

    if (p1.get() != 42) throw "p1 should be 42";
    if (*p2 != 1) throw "p2 should be 1";
    if (nz.get() != -5) throw "nz should be -5";
    if (nn.get() != 0) throw "nn should be 0";
    if (pct.get() != 50) throw "pct should be 50";

    return p1.get() + p2.get();
}

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

// ---- Test cases ----

TEST(CompileTime, IntConstruction) {
    constexpr int ct_result = test_compile_time_construction();
    static_assert(ct_result == 43);
    EXPECT_EQ(ct_result, 43);
}

TEST(CompileTime, FloatConstruction) {
    constexpr double ct_float_result = test_float_compile_time();
    static_assert(ct_float_result == 3.14);
    EXPECT_DOUBLE_EQ(ct_float_result, 3.14);
}

TEST(RuntimeConstruction, ValidPositiveInt) {
    PositiveInt p{42, runtime_check};
    EXPECT_EQ(p.get(), 42);
}

TEST(RuntimeConstruction, InvalidThrows) {
    EXPECT_THROW(PositiveInt(-1, runtime_check), refinement_error);
}

TEST(RuntimeConstruction, FloatTypes) {
    FiniteDouble fd{1.5, runtime_check};
    EXPECT_EQ(fd.get(), 1.5);

    NormalizedDouble nd{-0.5, runtime_check};
    EXPECT_EQ(nd.get(), -0.5);

    UnitDouble ud{0.5, runtime_check};
    EXPECT_EQ(ud.get(), 0.5);

    EXPECT_THROW(FiniteDouble(std::numeric_limits<double>::quiet_NaN(), runtime_check), refinement_error);
    EXPECT_THROW(FiniteDouble(std::numeric_limits<double>::infinity(), runtime_check), refinement_error);
    EXPECT_THROW(NormalizedDouble(2.0, runtime_check), refinement_error);
    EXPECT_THROW(UnitDouble(-0.1, runtime_check), refinement_error);
}

TEST(TryRefine, ValidAndInvalid) {
    auto maybe_positive = try_refine<PositiveInt>(42);
    ASSERT_TRUE(maybe_positive.has_value());
    EXPECT_EQ(maybe_positive->get(), 42);

    auto maybe_negative = try_refine<PositiveInt>(-1);
    EXPECT_FALSE(maybe_negative.has_value());

    auto maybe_even = try_refine<Even>(4);
    ASSERT_TRUE(maybe_even.has_value());
    EXPECT_EQ(maybe_even->get(), 4);

    auto maybe_odd_even = try_refine<Even>(3);
    EXPECT_FALSE(maybe_odd_even.has_value());
}

TEST(Predicates, Basic) {
    static_assert(Positive(5));
    static_assert(!Positive(-5));
    static_assert(!Positive(0));

    static_assert(NonZero(5));
    static_assert(NonZero(-5));
    static_assert(!NonZero(0));

    static_assert(NonNegative(0));
    static_assert(NonNegative(5));
    static_assert(!NonNegative(-5));

    constexpr auto in_0_100 = InRange(0, 100);
    static_assert(in_0_100(0));
    static_assert(in_0_100(50));
    static_assert(in_0_100(100));
    static_assert(!in_0_100(-1));
    static_assert(!in_0_100(101));

    constexpr auto gt_10 = GreaterThan(10);
    static_assert(gt_10(11));
    static_assert(!gt_10(10));
    static_assert(!gt_10(5));

    static_assert(Even(4));
    static_assert(!Even(3));
    static_assert(Odd(3));
    static_assert(!Odd(4));
    static_assert(DivisibleBy(3)(9));
    static_assert(!DivisibleBy(3)(10));
}

TEST(Predicates, Float) {
    static_assert(NotNaN(1.0));
    static_assert(NotNaN(0.0));
    static_assert(NotNaN(-1.0));
    EXPECT_FALSE(NotNaN(std::numeric_limits<double>::quiet_NaN()));

    EXPECT_TRUE(IsNaN(std::numeric_limits<double>::quiet_NaN()));
    EXPECT_TRUE(IsNaN(std::numeric_limits<float>::quiet_NaN()));
    static_assert(!IsNaN(1.0));
    static_assert(!IsNaN(0.0));

    static_assert(Finite(1.0));
    static_assert(Finite(0.0));
    static_assert(Finite(-1.0));
    EXPECT_FALSE(Finite(std::numeric_limits<double>::infinity()));
    EXPECT_FALSE(Finite(-std::numeric_limits<double>::infinity()));
    EXPECT_FALSE(Finite(std::numeric_limits<double>::quiet_NaN()));

    EXPECT_TRUE(IsInf(std::numeric_limits<double>::infinity()));
    EXPECT_TRUE(IsInf(-std::numeric_limits<double>::infinity()));
    static_assert(!IsInf(1.0));
    static_assert(!IsInf(0.0));
    EXPECT_FALSE(IsInf(std::numeric_limits<double>::quiet_NaN()));

    static_assert(IsNormal(1.0));
    static_assert(IsNormal(-1.0));
    static_assert(!IsNormal(0.0));

    constexpr auto near_zero = ApproxEqual(0.0, 0.001);
    static_assert(near_zero(0.0));
    static_assert(near_zero(0.0005));
    static_assert(near_zero(-0.0005));
    static_assert(!near_zero(0.01));
    static_assert(!near_zero(-0.01));

    constexpr auto near_pi = ApproxEqual(3.14159, 0.01);
    static_assert(near_pi(3.14));
    static_assert(!near_pi(3.0));
}

TEST(Composition, AllAnyNotIf) {
    constexpr auto positive_and_even = All<Positive, Even>;
    static_assert(positive_and_even(4));
    static_assert(!positive_and_even(-4));
    static_assert(!positive_and_even(3));

    constexpr auto positive_or_even = Any<Positive, Even>;
    static_assert(positive_or_even(3));
    static_assert(positive_or_even(-4));
    static_assert(!positive_or_even(-3));

    constexpr auto not_positive = Not<Positive>;
    static_assert(not_positive(-5));
    static_assert(not_positive(0));
    static_assert(!not_positive(5));

    constexpr auto even_implies_positive = If<Even, Positive>;
    static_assert(even_implies_positive(4));
    static_assert(!even_implies_positive(-4));
    static_assert(even_implies_positive(3));
}

TEST(Operations, SafeArithmetic) {
    constexpr NonZeroInt denom{2};
    constexpr int result = safe_divide(10, denom);
    static_assert(result == 5);

    constexpr auto abs_neg = refine::abs(-5);
    static_assert(abs_neg.get() == 5);
    static_assert(NonNegative(abs_neg.get()));

    constexpr auto sq = refine::square(-3);
    static_assert(sq.get() == 9);
    static_assert(NonNegative(sq.get()));

    constexpr PositiveInt a{5};
    constexpr PositiveInt b{3};
    constexpr auto min_ab = refined_min(a, b);
    static_assert(min_ab.get() == 3);
}

TEST(Operations, FloatMath) {
    NonNegativeDouble nn{4.0, runtime_check};
    auto sqrt_nn = refine::safe_sqrt(nn);
    EXPECT_NEAR(sqrt_nn.get(), 2.0, 1e-10);
    EXPECT_TRUE(NonNegative(sqrt_nn.get()));

    PositiveDouble pd{9.0, runtime_check};
    auto sqrt_pd = refine::safe_sqrt(pd);
    EXPECT_NEAR(sqrt_pd.get(), 3.0, 1e-10);
    EXPECT_TRUE(Positive(sqrt_pd.get()));

    NonNegativeDouble zero{0.0, runtime_check};
    auto sqrt_zero = refine::safe_sqrt(zero);
    EXPECT_DOUBLE_EQ(sqrt_zero.get(), 0.0);

    PositiveDouble e_val{std::numbers::e, runtime_check};
    double log_e = refine::safe_log(e_val);
    EXPECT_NEAR(log_e, 1.0, 1e-10);

    PositiveDouble one{1.0, runtime_check};
    double log_one = refine::safe_log(one);
    EXPECT_NEAR(log_one, 0.0, 1e-10);

    NormalizedDouble half{0.5, runtime_check};
    double asin_half = refine::safe_asin(half);
    EXPECT_NEAR(asin_half, std::asin(0.5), 1e-10);

    double acos_half = refine::safe_acos(half);
    EXPECT_NEAR(acos_half, std::acos(0.5), 1e-10);

    NonZeroDouble nz{4.0, runtime_check};
    double recip = refine::safe_reciprocal(nz);
    EXPECT_NEAR(recip, 0.25, 1e-10);

    NonZeroDouble neg_nz{-2.0, runtime_check};
    double recip_neg = refine::safe_reciprocal(neg_nz);
    EXPECT_NEAR(recip_neg, -0.5, 1e-10);
}

TEST(TypeAliases, All) {
    constexpr Percentage pct{75};
    static_assert(pct.get() == 75);

    constexpr Probability prob{0.5};
    static_assert(prob.get() == 0.5);

    constexpr ByteValue byte{255};
    static_assert(byte.get() == 255);

    constexpr PortNumber port{8080};
    static_assert(port.get() == 8080);
}

TEST(Conversion, ImplicitToUnderlying) {
    constexpr PositiveInt p{42};

    constexpr int i = p;
    static_assert(i == 42);

    auto square_int = [](int x) { return x * x; };
    int squared = square_int(p);
    EXPECT_EQ(squared, 1764);
}

TEST(Formatting, StdFormat) {
    PositiveInt p{42, runtime_check};
    std::string formatted = std::format("Value: {}", p);
    EXPECT_EQ(formatted, "Value: 42");
}

TEST(SafeArrayAccess, BoundedIndex) {
    constexpr int arr[] = {10, 20, 30, 40, 50};

    constexpr BoundedIndex<5> idx{2};
    constexpr int value = arr[idx.get()];
    static_assert(value == 30);
    EXPECT_EQ(value, 30);
}

TEST(Examples, SqrtPositive) {
    PositiveDouble pd{4.0, runtime_check};
    double result = sqrt_positive(pd);
    EXPECT_NEAR(result, 2.0, 0.1);
}

TEST(Comparisons, OrderingAndEquality) {
    constexpr PositiveInt a{5};
    constexpr PositiveInt b{3};
    constexpr PositiveInt c{5};

    static_assert(a == c);
    static_assert(!(a == b));
    static_assert(a > b);
    static_assert(b < a);
    static_assert(a >= c);
    static_assert(b <= a);

    static_assert(a == 5);
    static_assert(a > 3);
}

TEST(IsValid, StaticValidation) {
    static_assert(PositiveInt::is_valid(5));
    static_assert(!PositiveInt::is_valid(-5));
    static_assert(!PositiveInt::is_valid(0));

    static_assert(NonZeroInt::is_valid(5));
    static_assert(NonZeroInt::is_valid(-5));
    static_assert(!NonZeroInt::is_valid(0));
}

TEST(FloatEdgeCases, SpecialValues) {
    double neg_zero = -0.0;
    EXPECT_TRUE(NonNegative(neg_zero));
    EXPECT_TRUE(Finite(neg_zero));

    constexpr double max_val = std::numeric_limits<double>::max();
    static_assert(Finite(max_val));
    static_assert(Positive(max_val));

    constexpr double min_normal = std::numeric_limits<double>::min();
    static_assert(Positive(min_normal));
    static_assert(IsNormal(min_normal));

    constexpr double denorm = std::numeric_limits<double>::denorm_min();
    static_assert(Positive(denorm));
    static_assert(Finite(denorm));
    static_assert(!IsNormal(denorm));

    static_assert(Finite(1.0f));
    static_assert(Finite(1.0));
    EXPECT_FALSE(Finite(std::numeric_limits<float>::infinity()));
    EXPECT_FALSE(Finite(std::numeric_limits<double>::infinity()));

    FiniteFloat ff{1.5f, runtime_check};
    EXPECT_EQ(ff.get(), 1.5f);

    NormalizedFloat nf{-0.5f, runtime_check};
    EXPECT_EQ(nf.get(), -0.5f);
}

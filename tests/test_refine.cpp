// test_refine.cpp - Test suite for C++26 Refinement Types Library

#include <cmath>
#include <gtest/gtest.h>
#include <limits>
#include <numbers>
#include <refinery/domain.hpp>
#include <refinery/refinery.hpp>

using namespace refinery;

// ---- Helper templates used by tests ----

template <std::size_t N>
using BoundedIndex = Refined<std::size_t, InHalfOpenRange(std::size_t{0}, N)>;

template <typename T, std::size_t N>
constexpr const T& safe_at(const T (&arr)[N], BoundedIndex<N> index) {
    return arr[index.get()];
}

template <typename T>
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
    PositiveI32 p1{42};
    PositiveI32 p2{1};

    NonZeroI32 nz{-5};
    NonNegativeI32 nn{0};

    Percentage<> pct{50};

    if (p1.get() != 42)
        throw "p1 should be 42";
    if (*p2 != 1)
        throw "p2 should be 1";
    if (nz.get() != -5)
        throw "nz should be -5";
    if (nn.get() != 0)
        throw "nn should be 0";
    if (pct.get() != 50)
        throw "pct should be 50";

    return p1.get() + p2.get();
}

consteval double test_float_compile_time() {
    PositiveF64 pd{3.14};
    FiniteF64 fd{2.718};
    NormalizedF64 nd{0.5};
    UnitDouble<> ud{0.75};

    if (pd.get() != 3.14)
        throw "pd should be 3.14";
    if (fd.get() != 2.718)
        throw "fd should be 2.718";
    if (nd.get() != 0.5)
        throw "nd should be 0.5";
    if (ud.get() != 0.75)
        throw "ud should be 0.75";

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

TEST(RuntimeConstruction, ValidPositiveI32) {
    PositiveI32 p{42, runtime_check};
    EXPECT_EQ(p.get(), 42);
}

TEST(RuntimeConstruction, InvalidThrows) {
    EXPECT_THROW(PositiveI32(-1, runtime_check), refinement_error);
}

TEST(RuntimeConstruction, FloatTypes) {
    FiniteF64 fd{1.5, runtime_check};
    EXPECT_EQ(fd.get(), 1.5);

    NormalizedF64 nd{-0.5, runtime_check};
    EXPECT_EQ(nd.get(), -0.5);

    UnitDouble<> ud{0.5, runtime_check};
    EXPECT_EQ(ud.get(), 0.5);

    EXPECT_THROW(
        FiniteF64(std::numeric_limits<double>::quiet_NaN(), runtime_check),
        refinement_error);
    EXPECT_THROW(
        FiniteF64(std::numeric_limits<double>::infinity(), runtime_check),
        refinement_error);
    EXPECT_THROW(NormalizedF64(2.0, runtime_check), refinement_error);
    EXPECT_THROW((UnitDouble<>(-0.1, runtime_check)), refinement_error);
}

TEST(TryRefine, ValidAndInvalid) {
    auto maybe_positive = try_refine<PositiveI32>(42);
    ASSERT_TRUE(maybe_positive.has_value());
    EXPECT_EQ(maybe_positive->get(), 42);

    auto maybe_negative = try_refine<PositiveI32>(-1);
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
    constexpr NonZeroI32 denom{2};
    constexpr std::int32_t result = safe_divide(std::int32_t{10}, denom);
    static_assert(result == 5);

    constexpr auto abs_neg = refinery::abs(-5);
    static_assert(abs_neg.get() == 5);
    static_assert(NonNegative(abs_neg.get()));

    auto sq = refinery::square(-3);
    EXPECT_EQ(sq.get(), 9);
    EXPECT_TRUE(NonNegative(sq.get()));

    constexpr PositiveI32 a{5};
    constexpr PositiveI32 b{3};
    constexpr auto min_ab = refined_min(a, b);
    static_assert(min_ab.get() == 3);

    // Integer arithmetic now returns Refined directly (interval-based)
    auto int_sum = a + b;
    static_assert(interval_predicate<decltype(int_sum)::predicate>);
    EXPECT_EQ(int_sum.get(), 8);

    auto int_prod = a * b;
    static_assert(interval_predicate<decltype(int_prod)::predicate>);
    EXPECT_EQ(int_prod.get(), 15);

    // Float arithmetic returns Refined directly (no overflow to negative)
    PositiveF64 fa{5.0, runtime_check};
    PositiveF64 fb{3.0, runtime_check};
    auto float_sum = fa + fb;
    static_assert(std::same_as<decltype(float_sum), PositiveF64>);
    EXPECT_DOUBLE_EQ(float_sum.get(), 8.0);

    auto float_prod = fa * fb;
    static_assert(std::same_as<decltype(float_prod), PositiveF64>);
    EXPECT_DOUBLE_EQ(float_prod.get(), 15.0);
}

TEST(Operations, IntegerOverflow) {
    // abs(INT_MIN) throws
    EXPECT_THROW((void)refinery::abs(std::numeric_limits<int>::min()),
                 refinement_error);

    // abs of valid negative works
    auto abs_val = refinery::abs(-42);
    EXPECT_EQ(abs_val.get(), 42);

    // square(INT_MAX) throws (overflow)
    EXPECT_THROW((void)refinery::square(std::numeric_limits<int>::max()),
                 refinement_error);

    // square of small values works
    auto sq = refinery::square(100);
    EXPECT_EQ(sq.get(), 10000);

    // Float abs/square still work for extreme values
    auto abs_float = refinery::abs(-1.0e300);
    EXPECT_DOUBLE_EQ(abs_float.get(), 1.0e300);

    auto sq_float = refinery::square(1.0e300);
    EXPECT_TRUE(NonNegative(sq_float.get())); // inf, but still non-negative
}

TEST(Operations, FloatMath) {
    NonNegativeF64 nn{4.0, runtime_check};
    auto sqrt_nn = refinery::safe_sqrt(nn);
    EXPECT_NEAR(sqrt_nn.get(), 2.0, 1e-10);
    EXPECT_TRUE(NonNegative(sqrt_nn.get()));

    PositiveF64 pd{9.0, runtime_check};
    auto sqrt_pd = refinery::safe_sqrt(pd);
    EXPECT_NEAR(sqrt_pd.get(), 3.0, 1e-10);
    EXPECT_TRUE(Positive(sqrt_pd.get()));

    NonNegativeF64 zero{0.0, runtime_check};
    auto sqrt_zero = refinery::safe_sqrt(zero);
    EXPECT_DOUBLE_EQ(sqrt_zero.get(), 0.0);

    PositiveF64 e_val{std::numbers::e, runtime_check};
    double log_e = refinery::safe_log(e_val);
    EXPECT_NEAR(log_e, 1.0, 1e-10);

    PositiveF64 one{1.0, runtime_check};
    double log_one = refinery::safe_log(one);
    EXPECT_NEAR(log_one, 0.0, 1e-10);

    NormalizedF64 half{0.5, runtime_check};
    double asin_half = refinery::safe_asin(half);
    EXPECT_NEAR(asin_half, std::asin(0.5), 1e-10);

    double acos_half = refinery::safe_acos(half);
    EXPECT_NEAR(acos_half, std::acos(0.5), 1e-10);

    NonZeroF64 nz{4.0, runtime_check};
    double recip = refinery::safe_reciprocal(nz);
    EXPECT_NEAR(recip, 0.25, 1e-10);

    NonZeroF64 neg_nz{-2.0, runtime_check};
    double recip_neg = refinery::safe_reciprocal(neg_nz);
    EXPECT_NEAR(recip_neg, -0.5, 1e-10);
}

TEST(TypeAliases, All) {
    constexpr Percentage<> pct{75};
    static_assert(pct.get() == 75);

    constexpr Probability<> prob{0.5};
    static_assert(prob.get() == 0.5);

    constexpr ByteValue<> byte{255};
    static_assert(byte.get() == 255);

    constexpr PortNumber<> port{8080};
    static_assert(port.get() == 8080);
}

TEST(Conversion, ImplicitToUnderlying) {
    constexpr PositiveI32 p{42};

    constexpr int i = p;
    static_assert(i == 42);

    auto square_int = [](int x) { return x * x; };
    int squared = square_int(p);
    EXPECT_EQ(squared, 1764);
}

TEST(Formatting, StdFormat) {
    PositiveI32 p{42, runtime_check};
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
    PositiveF64 pd{4.0, runtime_check};
    double result = sqrt_positive(pd);
    EXPECT_NEAR(result, 2.0, 0.1);
}

TEST(Comparisons, OrderingAndEquality) {
    constexpr PositiveI32 a{5};
    constexpr PositiveI32 b{3};
    constexpr PositiveI32 c{5};

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
    static_assert(PositiveI32::is_valid(5));
    static_assert(!PositiveI32::is_valid(-5));
    static_assert(!PositiveI32::is_valid(0));

    static_assert(NonZeroI32::is_valid(5));
    static_assert(NonZeroI32::is_valid(-5));
    static_assert(!NonZeroI32::is_valid(0));
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

    FiniteF32 ff{1.5f, runtime_check};
    EXPECT_EQ(ff.get(), 1.5f);

    NormalizedF32 nf{-0.5f, runtime_check};
    EXPECT_EQ(nf.get(), -0.5f);
}

// ---- Interval Arithmetic Tests ----

TEST(Interval, PredicateBasics) {
    constexpr auto pred = Interval<0, 10>{};
    static_assert(pred(0));
    static_assert(pred(5));
    static_assert(pred(10));
    static_assert(!pred(-1));
    static_assert(!pred(11));

    static_assert(Interval<-3, 5>{}.lo == -3);
    static_assert(Interval<-3, 5>{}.hi == 5);
}

TEST(Interval, ConstevalConstruction) {
    constexpr IntervalRefined<int, 0, 10> x{5};
    static_assert(x.get() == 5);

    constexpr IntervalRefined<int, -3, 5> y{-2};
    static_assert(y.get() == -2);
}

TEST(Interval, RuntimeConstruction) {
    IntervalRefined<int, 0, 10> x{7, runtime_check};
    EXPECT_EQ(x.get(), 7);

    EXPECT_THROW((IntervalRefined<int, 0, 10>(11, runtime_check)),
                 refinement_error);
    EXPECT_THROW((IntervalRefined<int, 0, 10>(-1, runtime_check)),
                 refinement_error);
}

TEST(Interval, Addition) {
    constexpr IntervalRefined<int, 0, 10> a{3};
    constexpr IntervalRefined<int, -3, 5> b{2};

    constexpr auto result = a + b;
    static_assert(result.get() == 5);

    // Result type should be Interval<-3, 15>
    using ResultType = decltype(result);
    static_assert(
        std::same_as<ResultType, const Refined<int, Interval<-3, 15>{}>>);
}

TEST(Interval, Subtraction) {
    constexpr IntervalRefined<int, 0, 10> a{7};
    constexpr IntervalRefined<int, -3, 5> b{2};

    constexpr auto result = a - b;
    static_assert(result.get() == 5);

    // [0,10] - [-3,5] = [0-5, 10-(-3)] = [-5, 13]
    using ResultType = decltype(result);
    static_assert(
        std::same_as<ResultType, const Refined<int, Interval<-5, 13>{}>>);
}

TEST(Interval, Multiplication) {
    constexpr IntervalRefined<int, 1, 5> a{3};
    constexpr IntervalRefined<int, 2, 3> b{2};

    constexpr auto result = a * b;
    static_assert(result.get() == 6);

    // [1,5] * [2,3] = [2, 15]
    using ResultType = decltype(result);
    static_assert(
        std::same_as<ResultType, const Refined<int, Interval<2, 15>{}>>);
}

TEST(Interval, MultiplicationWithNegatives) {
    constexpr IntervalRefined<int, -2, 3> a{1};
    constexpr IntervalRefined<int, -1, 4> b{3};

    constexpr auto result = a * b;
    static_assert(result.get() == 3);

    // [-2,3] * [-1,4] = [min(2,-8,-3,12), max(2,-8,-3,12)] = [-8, 12]
    using ResultType = decltype(result);
    static_assert(
        std::same_as<ResultType, const Refined<int, Interval<-8, 12>{}>>);
}

TEST(Interval, Negation) {
    constexpr IntervalRefined<int, 2, 7> a{5};

    constexpr auto result = -a;
    static_assert(result.get() == -5);

    // -[2,7] = [-7, -2]
    using ResultType = decltype(result);
    static_assert(
        std::same_as<ResultType, const Refined<int, Interval<-7, -2>{}>>);
}

TEST(Interval, SameIntervalAddition) {
    // Two values with the same interval predicate should also work
    constexpr IntervalRefined<int, 0, 10> a{3};
    constexpr IntervalRefined<int, 0, 10> b{4};

    constexpr auto result = a + b;
    static_assert(result.get() == 7);

    // [0,10] + [0,10] = [0, 20]
    using ResultType = decltype(result);
    static_assert(
        std::same_as<ResultType, const Refined<int, Interval<0, 20>{}>>);
}

TEST(Interval, ChainedOperations) {
    constexpr IntervalRefined<int, 0, 10> a{3};
    constexpr IntervalRefined<int, -3, 5> b{2};
    constexpr IntervalRefined<int, 1, 2> c{2};

    // (a + b) * c
    constexpr auto sum = a + b; // Interval<-3, 15>
    constexpr auto result =
        sum *
        c; // [-3,15] * [1,2] = [min(-3,-6,15,30), max(-3,-6,15,30)] = [-6, 30]
    static_assert(result.get() == 10);

    using ResultType = decltype(result);
    static_assert(
        std::same_as<ResultType, const Refined<int, Interval<-6, 30>{}>>);
}

TEST(Interval, FloatingPoint) {
    constexpr IntervalRefined<double, 0.0, 1.0> a{0.5};
    constexpr IntervalRefined<double, -0.5, 0.5> b{0.25};

    constexpr auto result = a + b;
    static_assert(result.get() == 0.75);

    // [0.0, 1.0] + [-0.5, 0.5] = [-0.5, 1.5]
    using ResultType = decltype(result);
    static_assert(
        std::same_as<ResultType, const Refined<double, Interval<-0.5, 1.5>{}>>);
}

TEST(Interval, RuntimeArithmetic) {
    IntervalRefined<int, 0, 10> a{3, runtime_check};
    IntervalRefined<int, -3, 5> b{2, runtime_check};

    auto sum = a + b;
    EXPECT_EQ(sum.get(), 5);
    // Type check still works — the interval computation is compile-time even
    // with runtime values
    static_assert(
        std::same_as<decltype(sum), Refined<int, Interval<-3, 15>{}>>);

    auto diff = a - b;
    EXPECT_EQ(diff.get(), 1);
    static_assert(
        std::same_as<decltype(diff), Refined<int, Interval<-5, 13>{}>>);

    IntervalRefined<int, 1, 5> c{3, runtime_check};
    IntervalRefined<int, 2, 3> d{2, runtime_check};
    auto prod = c * d;
    EXPECT_EQ(prod.get(), 6);
    static_assert(
        std::same_as<decltype(prod), Refined<int, Interval<2, 15>{}>>);

    IntervalRefined<int, 2, 7> e{5, runtime_check};
    auto neg = -e;
    EXPECT_EQ(neg.get(), -5);
    static_assert(
        std::same_as<decltype(neg), Refined<int, Interval<-7, -2>{}>>);

    // Same-predicate runtime
    IntervalRefined<int, 0, 10> f{4, runtime_check};
    auto same_sum = a + f;
    EXPECT_EQ(same_sum.get(), 7);
    static_assert(
        std::same_as<decltype(same_sum), Refined<int, Interval<0, 20>{}>>);

    // Chained runtime
    auto chained = (a + b) * IntervalRefined<int, 1, 2>{2, runtime_check};
    EXPECT_EQ(chained.get(), 10);
    static_assert(
        std::same_as<decltype(chained), Refined<int, Interval<-6, 30>{}>>);
}

TEST(Interval, IntervalTraitsConcept) {
    static_assert(interval_predicate<Interval<0, 10>{}>);
    static_assert(interval_predicate<Interval<-5, 5>{}>);
    static_assert(!interval_predicate<Positive>);
    static_assert(!interval_predicate<NonZero>);
}

// ---- Composition Tests (S5/S6) ----

TEST(Composition, IffXor) {
    // Iff: both must have same truth value
    constexpr auto both_positive_or_both_not = Iff<Positive, NonZero>;
    static_assert(both_positive_or_both_not(5)); // both true
    static_assert(both_positive_or_both_not(0)); // both false → same
    static_assert(!both_positive_or_both_not(
        -5)); // Positive=false, NonZero=true → different

    // Xor: exactly one must be true
    constexpr auto one_but_not_both = Xor<Positive, Even>;
    static_assert(one_but_not_both(3));   // Positive=true, Even=false → true
    static_assert(one_but_not_both(-4));  // Positive=false, Even=true → true
    static_assert(!one_but_not_both(4));  // both true → false
    static_assert(!one_but_not_both(-3)); // both false → false
}

TEST(Composition, ExactlyNAtLeastNAtMostN) {
    constexpr auto exactly_two = ExactlyN<2, Positive, Even, NonZero>;
    static_assert(
        exactly_two(-2)); // Even=true, NonZero=true, Positive=false → 2
    static_assert(!exactly_two(2)); // all three true → 3
    static_assert(!exactly_two(0)); // Even=true → 1

    constexpr auto at_least_two = AtLeastN<2, Positive, Even, NonZero>;
    static_assert(at_least_two(2));  // all three → 3 >= 2
    static_assert(at_least_two(-2)); // Even+NonZero → 2 >= 2
    static_assert(!at_least_two(0)); // Even only → 1 < 2

    constexpr auto at_most_one = AtMostN<1, Positive, Even, NonZero>;
    static_assert(at_most_one(0));  // Even only → 1 <= 1
    static_assert(!at_most_one(2)); // all three → 3 > 1
}

TEST(Composition, ApplyOnMember) {
    struct Point {
        int x;
        int y;
    };

    constexpr auto x_positive = OnMember<&Point::x, Positive>;
    constexpr auto y_positive = OnMember<&Point::y, Positive>;

    static_assert(x_positive(Point{5, -3}));
    static_assert(!x_positive(Point{-1, 5}));
    static_assert(y_positive(Point{-1, 5}));
    static_assert(!y_positive(Point{5, -3}));

    // Apply with a projection function
    constexpr auto negate = [](int v) constexpr { return -v; };
    constexpr auto negate_is_positive = Apply<negate, Positive>;
    static_assert(negate_is_positive(-5)); // negate(-5) = 5 > 0
    static_assert(!negate_is_positive(5)); // negate(5) = -5 < 0
}

TEST(Composition, RuntimeComposition) {
    // runtime::AllOf
    runtime::AllOf<int> all_checks(Positive, NonZero);
    EXPECT_TRUE(all_checks(5));
    EXPECT_FALSE(all_checks(-5));
    EXPECT_FALSE(all_checks(0));

    // runtime::AnyOf
    runtime::AnyOf<int> any_checks(Positive, Even);
    EXPECT_TRUE(any_checks(3));   // Positive
    EXPECT_TRUE(any_checks(-4));  // Even
    EXPECT_FALSE(any_checks(-3)); // neither

    // runtime::NoneOf
    runtime::NoneOf<int> none_checks(Positive, Even);
    EXPECT_TRUE(none_checks(-3));  // neither positive nor even
    EXPECT_FALSE(none_checks(3));  // positive
    EXPECT_FALSE(none_checks(-4)); // even
}

TEST(Operations, TransformRefined) {
    PositiveI32 p{5, runtime_check};
    auto doubled =
        transform_refined<NonNegative>(p, [](std::int32_t v) { return v * 2; });
    EXPECT_EQ(doubled.get(), 10);
    static_assert(
        std::same_as<decltype(doubled), Refined<std::int32_t, NonNegative>>);
}

TEST(Operations, IncrementDecrement) {
    PositiveI32 p{1, runtime_check};

    auto inc = increment(p);
    ASSERT_TRUE(inc.has_value());
    EXPECT_EQ(inc->get(), 2);

    auto dec = decrement(p);
    // Decrementing 1 gives 0, which is not Positive
    EXPECT_FALSE(dec.has_value());

    NonNegativeI32 nn{0, runtime_check};
    auto dec_nn = decrement(nn);
    // Decrementing 0 gives -1, which is not NonNegative
    EXPECT_FALSE(dec_nn.has_value());

    auto inc_nn = increment(nn);
    ASSERT_TRUE(inc_nn.has_value());
    EXPECT_EQ(inc_nn->get(), 1);
}

TEST(Operations, RefinedClamp) {
    PositiveI32 lo{1, runtime_check};
    PositiveI32 hi{10, runtime_check};
    PositiveI32 val{5, runtime_check};

    auto clamped = refined_clamp(val, lo, hi);
    EXPECT_EQ(clamped.get(), 5);

    PositiveI32 below{1, runtime_check};
    PositiveI32 above{20, runtime_check};
    auto clamped_above = refined_clamp(above, lo, hi);
    EXPECT_EQ(clamped_above.get(), 10);
}

TEST(Concept, IsRefined) {
    static_assert(is_refined<PositiveI32>);
    static_assert(is_refined<NonZeroF64>);
    static_assert(is_refined<Percentage<>>);
    static_assert(!is_refined<int>);
    static_assert(!is_refined<double>);

    // Works with curried predicates (capturing lambdas)
    static_assert(is_refined<Refined<int, InRange(0, 100)>>);
    static_assert(is_refined<UnitDouble<>>);
}

// ---- Checked Arithmetic Tests ----

TEST(CheckedArith, AddNoOverflow) {
    using refinery::detail::checked_add;
    EXPECT_EQ(checked_add(3, 5), 8);
    EXPECT_EQ(checked_add(-3, -5), -8);
    EXPECT_EQ(checked_add(0, 0), 0);
}

TEST(CheckedArith, AddOverflow) {
    using refinery::detail::checked_add;
    EXPECT_THROW(checked_add(std::numeric_limits<int>::max(), 1),
                 refinement_error);
    EXPECT_THROW(checked_add(std::numeric_limits<int>::min(), -1),
                 refinement_error);
}

TEST(CheckedArith, SubNoOverflow) {
    using refinery::detail::checked_sub;
    EXPECT_EQ(checked_sub(10, 3), 7);
    EXPECT_EQ(checked_sub(-3, 5), -8);
}

TEST(CheckedArith, SubOverflow) {
    using refinery::detail::checked_sub;
    EXPECT_THROW(checked_sub(std::numeric_limits<int>::min(), 1),
                 refinement_error);
    EXPECT_THROW(checked_sub(std::numeric_limits<int>::max(), -1),
                 refinement_error);
}

TEST(CheckedArith, MulNoOverflow) {
    using refinery::detail::checked_mul;
    EXPECT_EQ(checked_mul(3, 5), 15);
    EXPECT_EQ(checked_mul(-3, 5), -15);
    EXPECT_EQ(checked_mul(0, std::numeric_limits<int>::max()), 0);
}

TEST(CheckedArith, MulOverflow) {
    using refinery::detail::checked_mul;
    EXPECT_THROW(checked_mul(std::numeric_limits<int>::max(), 2),
                 refinement_error);
    EXPECT_THROW(checked_mul(std::numeric_limits<int>::min(), 2),
                 refinement_error);
    // INT_MIN * -1 overflows (|INT_MIN| > INT_MAX)
    EXPECT_THROW(checked_mul(std::numeric_limits<int>::min(), -1),
                 refinement_error);
    EXPECT_THROW(checked_mul(-1, std::numeric_limits<int>::min()),
                 refinement_error);
}

TEST(CheckedArith, NegNoOverflow) {
    using refinery::detail::checked_neg;
    EXPECT_EQ(checked_neg(5), -5);
    EXPECT_EQ(checked_neg(-5), 5);
    EXPECT_EQ(checked_neg(0), 0);
}

TEST(CheckedArith, NegOverflow) {
    using refinery::detail::checked_neg;
    EXPECT_THROW(checked_neg(std::numeric_limits<int>::min()),
                 refinement_error);
}

TEST(SaturatingArith, EdgeCases) {
    using namespace refinery::interval_math::detail;
    constexpr auto imax = std::numeric_limits<int>::max();
    constexpr auto imin = std::numeric_limits<int>::min();

    // sat_add clamps on overflow
    static_assert(sat_add(imax, 1) == imax);
    static_assert(sat_add(imin, -1) == imin);
    static_assert(sat_add(3, 5) == 8);

    // sat_sub clamps on overflow
    static_assert(sat_sub(imin, 1) == imin);
    static_assert(sat_sub(imax, -1) == imax);
    static_assert(sat_sub(10, 3) == 7);

    // sat_mul clamps on overflow
    static_assert(sat_mul(imax, 2) == imax);
    static_assert(sat_mul(imin, 2) == imin);
    static_assert(sat_mul(imax, imax) == imax);
    static_assert(sat_mul(3, 5) == 15);
    static_assert(sat_mul(0, imax) == 0);

    // sat_neg: conservative for INT_MIN
    static_assert(sat_neg(imin) == imax);
    static_assert(sat_neg(-5) == 5);
    static_assert(sat_neg(0) == 0);
}

TEST(Interval, IntegerOverflowThrows) {
    // Addition that overflows
    IntervalRefined<int, 1, std::numeric_limits<int>::max()> big{
        std::numeric_limits<int>::max(), runtime_check};
    IntervalRefined<int, 1, 100> small{1, runtime_check};
    EXPECT_THROW((void)(big + small), refinement_error);

    // Multiplication that overflows
    IntervalRefined<int, 1, std::numeric_limits<int>::max()> big2{
        std::numeric_limits<int>::max(), runtime_check};
    IntervalRefined<int, 2, 3> two{2, runtime_check};
    EXPECT_THROW((void)(big2 * two), refinement_error);

    // Subtraction that overflows
    IntervalRefined<int, std::numeric_limits<int>::min(), -1> neg{
        std::numeric_limits<int>::min(), runtime_check};
    IntervalRefined<int, 1, 100> pos{1, runtime_check};
    EXPECT_THROW((void)(neg - pos), refinement_error);

    // Negation that overflows (negate INT_MIN)
    IntervalRefined<int, std::numeric_limits<int>::min(), -1> neg2{
        std::numeric_limits<int>::min(), runtime_check};
    EXPECT_THROW((void)(-neg2), refinement_error);
}

TEST(Interval, FloatNoThrow) {
    // Float operations should not throw even for extreme values
    IntervalRefined<double, 1.0, 1e308> big{1e308, runtime_check};
    IntervalRefined<double, 1.0, 1e308> big2{1e308, runtime_check};
    // This produces infinity but shouldn't throw
    auto result = big + big2;
    EXPECT_TRUE(std::isinf(result.get()));
    // The result interval upper bound is now infinity (not max()),
    // so the predicate should hold for the infinite value.
    constexpr auto pred = decltype(result)::predicate;
    EXPECT_TRUE(pred(result.get()));
}

// ---- Interval Alias Tests ----

TEST(IntervalAliases, PositiveI32IsInterval) {
    static_assert(interval_predicate<PositiveI32::predicate>);
    static_assert(decltype(PositiveI32::predicate)::lo == 1);
    static_assert(decltype(PositiveI32::predicate)::hi ==
                  std::numeric_limits<int>::max());

    constexpr PositiveI32 p{42};
    static_assert(p.get() == 42);

    PositiveI32 pr{42, runtime_check};
    EXPECT_EQ(pr.get(), 42);
    EXPECT_THROW(PositiveI32(0, runtime_check), refinement_error);
    EXPECT_THROW(PositiveI32(-1, runtime_check), refinement_error);
}

TEST(IntervalAliases, NonNegativeI32IsInterval) {
    static_assert(interval_predicate<NonNegativeI32::predicate>);
    static_assert(decltype(NonNegativeI32::predicate)::lo == 0);
    static_assert(decltype(NonNegativeI32::predicate)::hi ==
                  std::numeric_limits<int>::max());

    constexpr NonNegativeI32 z{0};
    static_assert(z.get() == 0);
    EXPECT_THROW(NonNegativeI32(-1, runtime_check), refinement_error);
}

TEST(IntervalAliases, ArithmeticReturnsRefined) {
    PositiveI32 a{5, runtime_check};
    PositiveI32 b{3, runtime_check};

    auto sum = a + b;
    EXPECT_EQ(sum.get(), 8);
    static_assert(interval_predicate<decltype(sum)::predicate>);

    auto prod = a * b;
    EXPECT_EQ(prod.get(), 15);
    static_assert(interval_predicate<decltype(prod)::predicate>);
}

TEST(IntervalAliases, NegationTransforms) {
    PositiveI32 a{5, runtime_check};
    auto neg = -a;
    EXPECT_EQ(neg.get(), -5);
    static_assert(interval_predicate<decltype(neg)::predicate>);
    static_assert(decltype(neg)::predicate.hi == -1);
}

TEST(IntervalAliases, SubtractionReturnsDegradedType) {
    PositiveI32 a{10, runtime_check};
    PositiveI32 b{3, runtime_check};

    // PositiveI32 - PositiveI32 has trivially-wide bounds, returns plain int
    auto diff = a - b;
    static_assert(std::same_as<decltype(diff), int>);
    EXPECT_EQ(diff, 7);

    // Narrow intervals still return Refined
    auto x = IntervalRefined<int, 1, 100>(50, runtime_check);
    auto y = IntervalRefined<int, 1, 100>(20, runtime_check);
    auto narrow_diff = x - y;
    static_assert(interval_predicate<decltype(narrow_diff)::predicate>);
    EXPECT_EQ(narrow_diff.get(), 30);
}

TEST(IntervalAliases, NegativeI32IsInterval) {
    static_assert(interval_predicate<NegativeI32::predicate>);
    static_assert(decltype(NegativeI32::predicate)::lo ==
                  std::numeric_limits<int>::min());
    static_assert(decltype(NegativeI32::predicate)::hi == -1);

    constexpr NegativeI32 n{-42};
    static_assert(n.get() == -42);

    NegativeI32 nr{-42, runtime_check};
    EXPECT_EQ(nr.get(), -42);
    EXPECT_THROW(NegativeI32(0, runtime_check), refinement_error);
    EXPECT_THROW(NegativeI32(1, runtime_check), refinement_error);
}

TEST(IntervalAliases, NonPositiveI32IsInterval) {
    static_assert(interval_predicate<NonPositiveI32::predicate>);
    static_assert(decltype(NonPositiveI32::predicate)::lo ==
                  std::numeric_limits<int>::min());
    static_assert(decltype(NonPositiveI32::predicate)::hi == 0);

    constexpr NonPositiveI32 z{0};
    static_assert(z.get() == 0);

    constexpr NonPositiveI32 n{-1};
    static_assert(n.get() == -1);
    EXPECT_THROW(NonPositiveI32(1, runtime_check), refinement_error);
}

TEST(IntervalAliases, NegativeArithmetic) {
    NegativeI32 a{-5, runtime_check};
    NegativeI32 b{-3, runtime_check};

    // NegativeI32 + NegativeI32: [-2*INT_MAX, -2], useful bounds -> Refined
    auto sum = a + b;
    EXPECT_EQ(sum.get(), -8);
    static_assert(interval_predicate<decltype(sum)::predicate>);
    static_assert(decltype(sum)::predicate.hi == -2);

    // Negation of NegativeI32 gives PositiveI32-like bounds
    auto neg = -a;
    EXPECT_EQ(neg.get(), 5);
    static_assert(interval_predicate<decltype(neg)::predicate>);
    static_assert(decltype(neg)::predicate.lo == 1);
}

TEST(IntervalAliases, NegativeToNonPositiveConversion) {
    // NegativeI32 ([INT_MIN, -1]) is a subset of NonPositiveI32 ([INT_MIN, 0])
    NegativeI32 n{-42, runtime_check};
    NonPositiveI32 np = n;
    EXPECT_EQ(np.get(), -42);
}

// ---- Predicate Implication Tests ----

TEST(Implication, IntervalToInterval) {
    // [1, 100] should convert to [0, 200]
    auto x = IntervalRefined<int, 1, 100>(42, runtime_check);
    IntervalRefined<int, 0, 200> wider = x;
    EXPECT_EQ(wider.get(), 42);

    // [1, 100] should convert to PositiveI32 (Interval<1, INT_MAX>)
    PositiveI32 p = x;
    EXPECT_EQ(p.get(), 42);

    // [1, 100] should convert to NonNegativeI32 (Interval<0, INT_MAX>)
    NonNegativeI32 nn = x;
    EXPECT_EQ(nn.get(), 42);
}

TEST(Implication, PredicateToPredicate) {
    // Positive implies NonZero (explicit trait)
    Refined<double, Positive> pd{3.14, runtime_check};
    Refined<double, NonZero> nz = pd;
    EXPECT_EQ(nz.get(), 3.14);

    // Positive implies NonNegative (explicit trait)
    Refined<double, NonNegative> nn = pd;
    EXPECT_EQ(nn.get(), 3.14);

    // Negative implies NonZero
    Refined<double, Negative> nd{-2.5, runtime_check};
    Refined<double, NonZero> nz2 = nd;
    EXPECT_EQ(nz2.get(), -2.5);

    // Negative implies NonPositive
    Refined<double, NonPositive> np = nd;
    EXPECT_EQ(np.get(), -2.5);
}

TEST(Implication, CompileTimeConversion) {
    constexpr auto x = IntervalRefined<int, 1, 100>(42);
    constexpr PositiveI32 p = x;
    static_assert(p.get() == 42);
}

TEST(Implication, InvalidConversionDoesNotCompile) {
    // [0, 100] does NOT imply Interval<1, INT_MAX> (0 < 1)
    static_assert(!detail::predicate_implies<
                  int, Interval<0, 100>{},
                  Interval<1, std::numeric_limits<int>::max()>{}>());
}

// ---- Float operator return type tests ----

TEST(Operations, FloatArithmeticUnchanged) {
    PositiveF64 a{5.0, runtime_check};
    PositiveF64 b{3.0, runtime_check};

    auto sum = a + b;
    static_assert(std::same_as<decltype(sum), PositiveF64>);
    EXPECT_DOUBLE_EQ(sum.get(), 8.0);

    auto prod = a * b;
    static_assert(std::same_as<decltype(prod), PositiveF64>);
    EXPECT_DOUBLE_EQ(prod.get(), 15.0);

    // Subtraction of floats with Positive predicate — result might not be
    // positive This now returns plain double (not optional)
    auto diff = a - b;
    static_assert(std::same_as<decltype(diff), double>);
    EXPECT_DOUBLE_EQ(diff, 2.0);

    // Negation of positive float — returns plain double
    auto neg = -a;
    static_assert(std::same_as<decltype(neg), double>);
    EXPECT_DOUBLE_EQ(neg, -5.0);
}

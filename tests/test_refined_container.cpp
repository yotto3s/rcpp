// test_refined_container.cpp - Tests for refined container wrappers

#include <array>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <refinery/refined_container.hpp>

using namespace refinery;

// --- SizeInterval predicate tests ---

TEST(SizeInterval, BasicPredicate) {
    constexpr auto pred = SizeInterval<3, 10>{};
    static_assert(pred(3));
    static_assert(pred(7));
    static_assert(pred(10));
    static_assert(!pred(2));
    static_assert(!pred(11));
    EXPECT_TRUE(pred(5));
}

TEST(SizeInterval, DefaultUpperBound) {
    // SizeInterval<5> means size >= 5 (upper bound is max)
    constexpr auto pred = SizeInterval<5>{};
    static_assert(pred(5));
    static_assert(pred(1000));
    static_assert(!pred(4));
    EXPECT_TRUE(pred(100));
}

TEST(SizeInterval, ZeroLowerBound) {
    // SizeInterval<0, 10> means size <= 10
    constexpr auto pred = SizeInterval<0, 10>{};
    static_assert(pred(0));
    static_assert(pred(10));
    static_assert(!pred(11));
    EXPECT_TRUE(pred(5));
}

TEST(SizeInterval, ExactSize) {
    // SizeInterval<5, 5> means size == 5
    constexpr auto pred = SizeInterval<5, 5>{};
    static_assert(pred(5));
    static_assert(!pred(4));
    static_assert(!pred(6));
    EXPECT_TRUE(pred(5));
}

TEST(SizeInterval, Traits) {
    using T = SizeInterval<3, 10>;
    static_assert(traits::size_interval_traits<T>::value);
    static_assert(traits::size_interval_traits<T>::lo == 3);
    static_assert(traits::size_interval_traits<T>::hi == 10);

    // Non-SizeInterval types should not match
    static_assert(!traits::size_interval_traits<int>::value);
}

TEST(SizeInterval, Concept) {
    static_assert(size_interval_predicate<SizeInterval<3, 10>{}>);
    static_assert(size_interval_predicate<SizeInterval<5>{}>);
}

// --- SizedContainer concept tests ---

TEST(SizedContainer, VectorSatisfies) {
    static_assert(SizedContainer<std::vector<int>>);
}

TEST(SizedContainer, ArraySatisfies) {
    static_assert(SizedContainer<std::array<int, 5>>);
}

// --- RefinedContainer construction tests ---

TEST(RefinedContainerConstruction, RuntimeCheckValid) {
    std::vector<int> v{1, 2, 3, 4, 5};
    RefinedContainer<std::vector<int>, SizeInterval<3>{}> rc(std::move(v),
                                                             runtime_check);
    EXPECT_EQ(rc.size(), 5);
    EXPECT_FALSE(rc.empty());
}

TEST(RefinedContainerConstruction, RuntimeCheckInvalid) {
    std::vector<int> v{1, 2};
    EXPECT_THROW(
        (RefinedContainer<std::vector<int>, SizeInterval<3>{}>(std::move(v),
                                                               runtime_check)),
        refinement_error);
}

TEST(RefinedContainerConstruction, AssumeValid) {
    std::vector<int> v{1, 2, 3};
    RefinedContainer<std::vector<int>, SizeInterval<3>{}> rc(std::move(v),
                                                             assume_valid);
    EXPECT_EQ(rc.size(), 3);
}

TEST(RefinedContainerConstruction, Get) {
    std::vector<int> v{10, 20, 30};
    RefinedContainer<std::vector<int>, SizeInterval<1>{}> rc(std::move(v),
                                                             runtime_check);
    const auto& inner = rc.get();
    EXPECT_EQ(inner.size(), 3);
    EXPECT_EQ(inner[0], 10);
}

TEST(RefinedContainerConstruction, Release) {
    std::vector<int> v{10, 20, 30};
    RefinedContainer<std::vector<int>, SizeInterval<1>{}> rc(std::move(v),
                                                             runtime_check);
    auto released = std::move(rc).release();
    EXPECT_EQ(released.size(), 3);
    EXPECT_EQ(released[1], 20);
}

// --- Iterator pass-through tests ---

TEST(RefinedContainerIterators, BeginEnd) {
    std::vector<int> v{1, 2, 3};
    RefinedContainer<std::vector<int>, SizeInterval<1>{}> rc(std::move(v),
                                                             runtime_check);
    int sum = 0;
    for (const auto& x : rc) {
        sum += x;
    }
    EXPECT_EQ(sum, 6);
}

TEST(RefinedContainerIterators, Data) {
    std::vector<int> v{10, 20, 30};
    RefinedContainer<std::vector<int>, SizeInterval<1>{}> rc(std::move(v),
                                                             runtime_check);
    const int* p = rc.data();
    EXPECT_EQ(p[0], 10);
    EXPECT_EQ(p[2], 30);
}

// --- Predicate-gated access tests ---

TEST(RefinedContainerGatedAccess, FrontBackWithNonEmpty) {
    std::vector<int> v{10, 20, 30};
    // SizeInterval<1> guarantees non-empty -> front()/back() should compile
    RefinedContainer<std::vector<int>, SizeInterval<1>{}> rc(std::move(v),
                                                             runtime_check);
    EXPECT_EQ(rc.front(), 10);
    EXPECT_EQ(rc.back(), 30);
}

TEST(RefinedContainerGatedAccess, FrontBackWithExactSize) {
    std::vector<int> v{42};
    // SizeInterval<1, 1> guarantees exactly 1 element
    RefinedContainer<std::vector<int>, SizeInterval<1, 1>{}> rc(std::move(v),
                                                                runtime_check);
    EXPECT_EQ(rc.front(), 42);
    EXPECT_EQ(rc.back(), 42);
}

// Compile-time check: front()/back() compile when size >= 1 is provable
static_assert(
    requires(RefinedContainer<std::vector<int>, SizeInterval<1>{}> rc) {
        rc.front();
        rc.back();
    });

// --- Predicate propagation trait tests ---

TEST(SizeIntervalShift, PushBack) {
    // SizeInterval<3, 10> + 1 -> SizeInterval<4, 11>
    constexpr auto shifted = size_interval_shift<SizeInterval<3, 10>{}, 1>();
    static_assert(shifted.lo == 4);
    static_assert(shifted.hi == 11);
}

TEST(SizeIntervalShift, PopBack) {
    // SizeInterval<3, 10> - 1 -> SizeInterval<2, 9>
    constexpr auto shifted = size_interval_shift<SizeInterval<3, 10>{}, -1>();
    static_assert(shifted.lo == 2);
    static_assert(shifted.hi == 9);
}

TEST(SizeIntervalShift, DefaultUpperBound) {
    // SizeInterval<5> + 1 -> SizeInterval<6> (hi stays at max)
    constexpr auto shifted = size_interval_shift<SizeInterval<5>{}, 1>();
    static_assert(shifted.lo == 6);
    static_assert(shifted.hi == std::numeric_limits<std::size_t>::max());
}

TEST(SizeIntervalShift, MultipleElements) {
    // SizeInterval<2> + 3 -> SizeInterval<5>
    constexpr auto shifted = size_interval_shift<SizeInterval<2>{}, 3>();
    static_assert(shifted.lo == 5);
}

// --- Mutation tests ---

TEST(RefinedContainerMutation, PushBack) {
    std::vector<int> v{1, 2, 3};
    RefinedContainer<std::vector<int>, SizeInterval<3>{}> rc(std::move(v),
                                                             runtime_check);

    auto rc2 = std::move(rc).push_back(4);
    EXPECT_EQ(rc2.size(), 4);
    EXPECT_EQ(rc2.back(), 4);

    // Verify the predicate was updated: rc2 should have SizeInterval<4>
    static_assert(std::same_as<
                  decltype(rc2),
                  RefinedContainer<std::vector<int>, SizeInterval<4>{}>>);
}

TEST(RefinedContainerMutation, PopBack) {
    std::vector<int> v{1, 2, 3};
    RefinedContainer<std::vector<int>, SizeInterval<3>{}> rc(std::move(v),
                                                             runtime_check);

    auto rc2 = std::move(rc).pop_back();
    EXPECT_EQ(rc2.size(), 2);

    // Verify predicate: SizeInterval<3> - 1 -> SizeInterval<2>
    static_assert(std::same_as<
                  decltype(rc2),
                  RefinedContainer<std::vector<int>, SizeInterval<2>{}>>);
}

TEST(RefinedContainerMutation, EmplaceBack) {
    std::vector<std::string> v{"hello"};
    RefinedContainer<std::vector<std::string>, SizeInterval<1>{}> rc(
        std::move(v), runtime_check);

    auto rc2 = std::move(rc).emplace_back("world");
    EXPECT_EQ(rc2.size(), 2);
    EXPECT_EQ(rc2.back(), "world");

    static_assert(std::same_as<
                  decltype(rc2),
                  RefinedContainer<std::vector<std::string>, SizeInterval<2>{}>>);
}

TEST(RefinedContainerMutation, ChainedPushBack) {
    std::vector<int> v;
    RefinedContainer<std::vector<int>, SizeInterval<0>{}> rc(std::move(v),
                                                             runtime_check);

    auto rc2 = std::move(rc).push_back(1).push_back(2).push_back(3);
    EXPECT_EQ(rc2.size(), 3);
    EXPECT_EQ(rc2.front(), 1);

    static_assert(std::same_as<
                  decltype(rc2),
                  RefinedContainer<std::vector<int>, SizeInterval<3>{}>>);
}

// --- Static indexing tests ---

#include <refinery/refinery.hpp>

TEST(RefinedContainerIndex, StaticBoundsAccess) {
    std::vector<int> v{10, 20, 30, 40, 50};
    RefinedContainer<std::vector<int>, SizeInterval<5>{}> rc(std::move(v),
                                                             runtime_check);

    // Interval<0, 4> has upper bound 4 < lower bound 5 -> compiles
    using Idx = Refined<std::size_t,
                        Interval<std::size_t{0}, std::size_t{4}>{}>;
    Idx idx{3};
    EXPECT_EQ(rc[idx], 40);
}

TEST(RefinedContainerIndex, ZeroIndex) {
    std::vector<int> v{42, 99};
    RefinedContainer<std::vector<int>, SizeInterval<2>{}> rc(std::move(v),
                                                             runtime_check);

    using Idx = Refined<std::size_t,
                        Interval<std::size_t{0}, std::size_t{1}>{}>;
    Idx idx{0};
    EXPECT_EQ(rc[idx], 42);
}

// Compile-time: index with upper bound < container lower bound should work
static_assert(
    requires(RefinedContainer<std::vector<int>, SizeInterval<5>{}> rc,
             Refined<std::size_t,
                     Interval<std::size_t{0}, std::size_t{4}>{}> idx) {
        rc[idx]; // upper bound 4 < lower bound 5 -> should compile
    });

// --- Range insertion tests ---

TEST(RefinedContainerAppend, FromArray) {
    std::vector<int> v{1, 2, 3};
    RefinedContainer<std::vector<int>, SizeInterval<3>{}> rc(std::move(v),
                                                             runtime_check);

    std::array<int, 2> arr{4, 5};
    auto rc2 = std::move(rc).append(arr);
    EXPECT_EQ(rc2.size(), 5);
    EXPECT_EQ(rc2.back(), 5);

    // SizeInterval<3> + 2 -> SizeInterval<5>
    static_assert(std::same_as<
                  decltype(rc2),
                  RefinedContainer<std::vector<int>, SizeInterval<5>{}>>);
}

TEST(RefinedContainerAppend, FromRefinedContainer) {
    std::vector<int> target_v{1, 2, 3};
    RefinedContainer<std::vector<int>, SizeInterval<3>{}> target(
        std::move(target_v), runtime_check);

    std::vector<int> source_v{4, 5};
    RefinedContainer<std::vector<int>, SizeInterval<2>{}> source(
        std::move(source_v), runtime_check);

    // SizeInterval<3> + lower_bound(SizeInterval<2>) -> SizeInterval<5>
    auto result = std::move(target).append(std::move(source));
    EXPECT_EQ(result.size(), 5);

    static_assert(std::same_as<
                  decltype(result),
                  RefinedContainer<std::vector<int>, SizeInterval<5>{}>>);
}

TEST(RefinedContainerAppend, FromEmptyArray) {
    std::vector<int> v{1};
    RefinedContainer<std::vector<int>, SizeInterval<1>{}> rc(std::move(v),
                                                             runtime_check);

    std::array<int, 0> arr{};
    auto rc2 = std::move(rc).append(arr);
    EXPECT_EQ(rc2.size(), 1);

    // SizeInterval<1> + 0 -> SizeInterval<1>
    static_assert(std::same_as<
                  decltype(rc2),
                  RefinedContainer<std::vector<int>, SizeInterval<1>{}>>);
}

// --- Freeze/guard tests ---

TEST(FreezeGuard, BasicUsage) {
    std::vector<int> v{10, 20, 30, 40, 50};
    RefinedContainer<std::vector<int>, SizeInterval<1>{}> rc(std::move(v),
                                                             runtime_check);

    auto [guard, frozen] = std::move(rc).freeze();

    // Valid index
    auto idx = guard.check(2);
    ASSERT_TRUE(idx.has_value());
    EXPECT_EQ(frozen[*idx], 30);
}

TEST(FreezeGuard, OutOfBounds) {
    std::vector<int> v{10, 20, 30};
    RefinedContainer<std::vector<int>, SizeInterval<1>{}> rc(std::move(v),
                                                             runtime_check);

    auto [guard, frozen] = std::move(rc).freeze();

    // Out-of-bounds index
    auto idx = guard.check(5);
    EXPECT_FALSE(idx.has_value());
}

TEST(FreezeGuard, BoundaryIndex) {
    std::vector<int> v{10, 20, 30};
    RefinedContainer<std::vector<int>, SizeInterval<1>{}> rc(std::move(v),
                                                             runtime_check);

    auto [guard, frozen] = std::move(rc).freeze();

    // Last valid index
    auto last = guard.check(2);
    ASSERT_TRUE(last.has_value());
    EXPECT_EQ(frozen[*last], 30);

    // One past end
    auto past = guard.check(3);
    EXPECT_FALSE(past.has_value());
}

TEST(FreezeGuard, IterationThroughGuard) {
    std::vector<int> v{1, 2, 3, 4, 5};
    RefinedContainer<std::vector<int>, SizeInterval<1>{}> rc(std::move(v),
                                                             runtime_check);

    auto [guard, frozen] = std::move(rc).freeze();

    int sum = 0;
    for (std::size_t i = 0;; ++i) {
        auto idx = guard.check(i);
        if (!idx)
            break;
        sum += frozen[*idx];
    }
    EXPECT_EQ(sum, 15);
}

// Compile-time branding: indices from different freeze() calls are
// incompatible
TEST(FreezeGuard, BrandingPreventsTypeMismatch) {
    // Two different freeze() call sites produce different Tag types
    std::vector<int> v1{10, 20};
    RefinedContainer<std::vector<int>, SizeInterval<1>{}> rc1(std::move(v1),
                                                              runtime_check);
    auto [guard1, frozen1] = std::move(rc1).freeze();

    std::vector<int> v2{30, 40, 50};
    RefinedContainer<std::vector<int>, SizeInterval<1>{}> rc2(std::move(v2),
                                                              runtime_check);
    auto [guard2, frozen2] = std::move(rc2).freeze();

    // guard1 and guard2 should produce different GuardedIndex types
    auto idx1 = guard1.check(0);
    auto idx2 = guard2.check(0);
    ASSERT_TRUE(idx1.has_value());
    ASSERT_TRUE(idx2.has_value());

    // These should work (matching brands)
    EXPECT_EQ(frozen1[*idx1], 10);
    EXPECT_EQ(frozen2[*idx2], 30);

    // The following should NOT compile (mismatched brands):
    // frozen1[*idx2];  // error: different Tag types
    // frozen2[*idx1];  // error: different Tag types

    // Verify the types are actually different
    static_assert(!std::same_as<decltype(*idx1), decltype(*idx2)>);
}

TEST(FreezeGuard, FrozenContainerSize) {
    std::vector<int> v{1, 2, 3};
    RefinedContainer<std::vector<int>, SizeInterval<1>{}> rc(std::move(v),
                                                             runtime_check);

    auto [guard, frozen] = std::move(rc).freeze();
    EXPECT_EQ(frozen.size(), 3);
}

// test_refined_container.cpp - Tests for refined container wrappers

#include <array>
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

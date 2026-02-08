// test_refined_container.cpp - Tests for refined container wrappers

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

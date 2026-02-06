// refined.hpp - Main header for C++26 Refinement Types Library
// Part of the C++26 Refinement Types Library
//
// This library provides Liquid Haskell-style refinement types for C++26,
// using GCC's reflection implementation for rich compile-time diagnostics.
//
// Example usage:
//
//   #include <refinery/refinery.hpp>
//   using namespace refinery;
//
//   // Type-safe division - denominator is guaranteed non-zero
//   template<typename T>
//   constexpr T safe_divide(T num, Refined<T, NonZero> denom) {
//       return num / denom.get();  // Can never divide by zero!
//   }
//
//   // Compile-time verification
//   consteval int demo() {
//       PositiveInt x{42};      // OK
//       // PositiveInt y{-1};   // COMPILE ERROR with rich message
//       return safe_divide(100, NonZeroInt{2});
//   }
//
//   // Runtime validation
//   void process(int user_input) {
//       if (auto validated = try_refine<PositiveInt>(user_input)) {
//           use(*validated);
//       } else {
//           handle_invalid_input();
//       }
//   }

#ifndef REFINERY_REFINERY_HPP
#define REFINERY_REFINERY_HPP

#include "compose.hpp"
#include "diagnostics.hpp"
#include "interval.hpp"
#include "operations.hpp"
#include "predicates.hpp"
#include "refined_type.hpp"

namespace refinery {

// Common refined type aliases

// Positive integers (> 0) — interval-based for arithmetic ergonomics
using PositiveInt = IntervalRefined<int, 1, std::numeric_limits<int>::max()>;
using PositiveLong =
    IntervalRefined<long, 1L, std::numeric_limits<long>::max()>;
using PositiveLongLong =
    IntervalRefined<long long, 1LL, std::numeric_limits<long long>::max()>;

// Negative integers (< 0) — interval-based
using NegativeInt = IntervalRefined<int, std::numeric_limits<int>::min(), -1>;
using NegativeLong =
    IntervalRefined<long, std::numeric_limits<long>::min(), -1L>;
using NegativeLongLong =
    IntervalRefined<long long, std::numeric_limits<long long>::min(), -1LL>;

// Non-negative integers (>= 0) — interval-based
using NonNegativeInt = IntervalRefined<int, 0, std::numeric_limits<int>::max()>;
using NonNegativeLong =
    IntervalRefined<long, 0L, std::numeric_limits<long>::max()>;
using NonNegativeLongLong =
    IntervalRefined<long long, 0LL, std::numeric_limits<long long>::max()>;

// Non-positive integers (<= 0) — interval-based
using NonPositiveInt = IntervalRefined<int, std::numeric_limits<int>::min(), 0>;
using NonPositiveLong =
    IntervalRefined<long, std::numeric_limits<long>::min(), 0L>;
using NonPositiveLongLong =
    IntervalRefined<long long, std::numeric_limits<long long>::min(), 0LL>;

// Non-zero integers (!= 0) — predicate-based (cannot be a single interval).
// Arithmetic on NonZero types returns plain T, not interval-refined results.
using NonZeroInt = Refined<int, NonZero>;
using NonZeroLong = Refined<long, NonZero>;
using NonZeroLongLong = Refined<long long, NonZero>;

// Positive floating point
using PositiveFloat = Refined<float, Positive>;
using PositiveDouble = Refined<double, Positive>;

// Non-negative floating point
using NonNegativeFloat = Refined<float, NonNegative>;
using NonNegativeDouble = Refined<double, NonNegative>;

// Non-zero floating point
using NonZeroFloat = Refined<float, NonZero>;
using NonZeroDouble = Refined<double, NonZero>;

// Percentage type (0-100)
inline constexpr auto IsPercentage = InRange(0, 100);
using Percentage = Refined<int, IsPercentage>;

// Probability type (0.0-1.0)
inline constexpr auto IsProbability = [](double v) constexpr {
    return v >= 0.0 && v <= 1.0;
};
using Probability = Refined<double, IsProbability>;

// Finite floating point
using FiniteFloat = Refined<float, Finite>;
using FiniteDouble = Refined<double, Finite>;

// Normalized floating point [-1, 1]
using NormalizedFloat = Refined<float, Normalized>;
using NormalizedDouble = Refined<double, Normalized>;

// Unit interval [0, 1]
inline constexpr auto IsUnit = [](auto v) constexpr {
    return v >= decltype(v){0} && v <= decltype(v){1};
};
using UnitFloat = Refined<float, IsUnit>;
using UnitDouble = Refined<double, IsUnit>;

// Byte value (0-255)
inline constexpr auto IsByte = InRange(0, 255);
using ByteValue = Refined<int, IsByte>;

// Port number (1-65535)
inline constexpr auto IsPort = InRange(1, 65535);
using PortNumber = Refined<int, IsPort>;

// Natural numbers (positive integers)
using Natural = PositiveInt;

// Whole numbers (non-negative integers)
using Whole = NonNegativeInt;

// Helper to create a named predicate with documentation
#define DEFINE_PREDICATE(Name, ...) inline constexpr auto Name = __VA_ARGS__

} // namespace refinery

#endif // REFINERY_REFINERY_HPP

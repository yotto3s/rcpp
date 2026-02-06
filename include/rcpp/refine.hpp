// refine.hpp - Main header for C++26 Refinement Types Library
// Part of the C++26 Refinement Types Library
//
// This library provides Liquid Haskell-style refinement types for C++26,
// using GCC's reflection implementation for rich compile-time diagnostics.
//
// Example usage:
//
//   #include <rcpp/refine.hpp>
//   using namespace refine;
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

#ifndef RCPP_REFINE_HPP
#define RCPP_REFINE_HPP

#include "diagnostics.hpp"
#include "predicates.hpp"
#include "compose.hpp"
#include "refined.hpp"
#include "operations.hpp"

namespace refine {

// Common refined type aliases

// Positive integers (> 0)
using PositiveInt = Refined<int, Positive>;
using PositiveLong = Refined<long, Positive>;
using PositiveLongLong = Refined<long long, Positive>;

// Non-negative integers (>= 0)
using NonNegativeInt = Refined<int, NonNegative>;
using NonNegativeLong = Refined<long, NonNegative>;
using NonNegativeLongLong = Refined<long long, NonNegative>;

// Non-zero integers (!= 0)
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
inline constexpr auto IsUnit = InRange(0.0, 1.0);
using UnitFloat = Refined<float, IsUnit>;
using UnitDouble = Refined<double, IsUnit>;

// Byte value (0-255)
inline constexpr auto IsByte = InRange(0, 255);
using ByteValue = Refined<int, IsByte>;

// Port number (1-65535)
inline constexpr auto IsPort = InRange(1, 65535);
using PortNumber = Refined<int, IsPort>;

// Array index type (non-negative size_t)
using Index = Refined<std::size_t, NonNegative>;

// Natural numbers (positive integers)
using Natural = PositiveInt;

// Whole numbers (non-negative integers)
using Whole = NonNegativeInt;

// Helper macro for defining custom refined types
// Usage: DEFINE_REFINED_TYPE(PositiveInt, int, [](int v) { return v > 0; })
#define DEFINE_REFINED_TYPE(Name, Type, Pred) \
    inline constexpr auto Name##Predicate = Pred; \
    using Name = ::refine::Refined<Type, Name##Predicate>

// Helper to create a named predicate with documentation
#define DEFINE_PREDICATE(Name, ...) \
    inline constexpr auto Name = __VA_ARGS__

} // namespace refine

#endif // RCPP_REFINE_HPP

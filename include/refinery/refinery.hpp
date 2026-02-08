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
//   consteval std::int32_t demo() {
//       PositiveI32 x{42};      // OK
//       // PositiveI32 y{-1};   // COMPILE ERROR with rich message
//       return safe_divide(std::int32_t{100}, NonZeroI32{2});
//   }
//
//   // Runtime validation
//   void process(int user_input) {
//       if (auto validated = try_refine<PositiveI32>(user_input)) {
//           use(*validated);
//       } else {
//           handle_invalid_input();
//       }
//   }

#ifndef REFINERY_REFINERY_HPP
#define REFINERY_REFINERY_HPP

#include <cstddef>
#include <cstdint>
#include <limits>

#include "compose.hpp"
#include "diagnostics.hpp"
#include "interval.hpp"
#include "operations.hpp"
#include "predicates.hpp"
#include "refined_container.hpp"
#include "refined_type.hpp"

namespace refinery {

// --- Signed integers ---

// Positive integers (> 0) — interval-based for arithmetic ergonomics
using PositiveI8 = IntervalRefined<std::int8_t, std::int8_t{1},
                                   std::numeric_limits<std::int8_t>::max()>;
using PositiveI16 = IntervalRefined<std::int16_t, std::int16_t{1},
                                    std::numeric_limits<std::int16_t>::max()>;
using PositiveI32 = IntervalRefined<std::int32_t, std::int32_t{1},
                                    std::numeric_limits<std::int32_t>::max()>;
using PositiveI64 = IntervalRefined<std::int64_t, std::int64_t{1},
                                    std::numeric_limits<std::int64_t>::max()>;

// Negative integers (< 0) — interval-based
using NegativeI8 =
    IntervalRefined<std::int8_t, std::numeric_limits<std::int8_t>::min(),
                    std::int8_t{-1}>;
using NegativeI16 =
    IntervalRefined<std::int16_t, std::numeric_limits<std::int16_t>::min(),
                    std::int16_t{-1}>;
using NegativeI32 =
    IntervalRefined<std::int32_t, std::numeric_limits<std::int32_t>::min(),
                    std::int32_t{-1}>;
using NegativeI64 =
    IntervalRefined<std::int64_t, std::numeric_limits<std::int64_t>::min(),
                    std::int64_t{-1}>;

// Non-negative integers (>= 0) — interval-based
using NonNegativeI8 = IntervalRefined<std::int8_t, std::int8_t{0},
                                      std::numeric_limits<std::int8_t>::max()>;
using NonNegativeI16 =
    IntervalRefined<std::int16_t, std::int16_t{0},
                    std::numeric_limits<std::int16_t>::max()>;
using NonNegativeI32 =
    IntervalRefined<std::int32_t, std::int32_t{0},
                    std::numeric_limits<std::int32_t>::max()>;
using NonNegativeI64 =
    IntervalRefined<std::int64_t, std::int64_t{0},
                    std::numeric_limits<std::int64_t>::max()>;

// Non-positive integers (<= 0) — interval-based
using NonPositiveI8 =
    IntervalRefined<std::int8_t, std::numeric_limits<std::int8_t>::min(),
                    std::int8_t{0}>;
using NonPositiveI16 =
    IntervalRefined<std::int16_t, std::numeric_limits<std::int16_t>::min(),
                    std::int16_t{0}>;
using NonPositiveI32 =
    IntervalRefined<std::int32_t, std::numeric_limits<std::int32_t>::min(),
                    std::int32_t{0}>;
using NonPositiveI64 =
    IntervalRefined<std::int64_t, std::numeric_limits<std::int64_t>::min(),
                    std::int64_t{0}>;

// Non-zero integers (!= 0) — predicate-based (cannot be a single interval).
// Arithmetic on NonZero types returns plain T, not interval-refined results.
using NonZeroI8 = Refined<std::int8_t, NonZero>;
using NonZeroI16 = Refined<std::int16_t, NonZero>;
using NonZeroI32 = Refined<std::int32_t, NonZero>;
using NonZeroI64 = Refined<std::int64_t, NonZero>;

// --- Unsigned integers (NonZero only — Positive ≡ NonZero, NonNegative always
// true) ---

using NonZeroU8 = Refined<std::uint8_t, NonZero>;
using NonZeroU16 = Refined<std::uint16_t, NonZero>;
using NonZeroU32 = Refined<std::uint32_t, NonZero>;
using NonZeroU64 = Refined<std::uint64_t, NonZero>;

// --- Size type (NonZero only — same reasoning as unsigned) ---

using NonZeroUsize = Refined<std::size_t, NonZero>;

// --- Floating point ---

using PositiveF32 = Refined<float, Positive>;
using PositiveF64 = Refined<double, Positive>;

using NegativeF32 = Refined<float, Negative>;
using NegativeF64 = Refined<double, Negative>;

using NonNegativeF32 = Refined<float, NonNegative>;
using NonNegativeF64 = Refined<double, NonNegative>;

using NonPositiveF32 = Refined<float, NonPositive>;
using NonPositiveF64 = Refined<double, NonPositive>;

using NonZeroF32 = Refined<float, NonZero>;
using NonZeroF64 = Refined<double, NonZero>;

using FiniteF32 = Refined<float, Finite>;
using FiniteF64 = Refined<double, Finite>;

using NormalizedF32 = Refined<float, Normalized>;
using NormalizedF64 = Refined<double, Normalized>;

} // namespace refinery

#endif // REFINERY_REFINERY_HPP

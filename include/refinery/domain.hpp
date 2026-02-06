#ifndef REFINERY_DOMAIN_HPP
#define REFINERY_DOMAIN_HPP

#include "refinery.hpp"

namespace refinery {

// Percentage type (0-100)
inline constexpr auto IsPercentage = InRange(0, 100);
template <typename T = std::int32_t>
using Percentage = Refined<T, IsPercentage>;

// Probability type (0.0-1.0)
inline constexpr auto IsProbability = [](auto v) constexpr {
    return v >= decltype(v){0} && v <= decltype(v){1};
};
template <typename T = double> using Probability = Refined<T, IsProbability>;

// Unit interval [0, 1]
inline constexpr auto IsUnit = [](auto v) constexpr {
    return v >= decltype(v){0} && v <= decltype(v){1};
};
template <typename T = float> using UnitFloat = Refined<T, IsUnit>;
template <typename T = double> using UnitDouble = Refined<T, IsUnit>;

// Byte value (0-255)
inline constexpr auto IsByte = InRange(0, 255);
template <typename T = std::int32_t> using ByteValue = Refined<T, IsByte>;

// Port number (1-65535)
inline constexpr auto IsPort = InRange(1, 65535);
template <typename T = std::int32_t> using PortNumber = Refined<T, IsPort>;

// Natural numbers (positive integers)
using Natural = PositiveI32;

// Whole numbers (non-negative integers)
using Whole = NonNegativeI32;

} // namespace refinery

#endif // REFINERY_DOMAIN_HPP

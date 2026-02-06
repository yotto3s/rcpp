// predicates.hpp - Standard predicates for refinement types
// Part of the C++26 Refinement Types Library

#ifndef RCPP_PREDICATES_HPP
#define RCPP_PREDICATES_HPP

#include <concepts>
#include <type_traits>
#include <cstddef>
#include <string>
#include <string_view>

namespace refine {

// Basic numeric predicates
inline constexpr auto Positive = [](auto v) constexpr {
    return v > decltype(v){0};
};

inline constexpr auto Negative = [](auto v) constexpr {
    return v < decltype(v){0};
};

inline constexpr auto NonNegative = [](auto v) constexpr {
    return v >= decltype(v){0};
};

inline constexpr auto NonPositive = [](auto v) constexpr {
    return v <= decltype(v){0};
};

inline constexpr auto NonZero = [](auto v) constexpr {
    return v != decltype(v){0};
};

inline constexpr auto Zero = [](auto v) constexpr {
    return v == decltype(v){0};
};

// Range predicates (curried)
inline constexpr auto GreaterThan = [](auto bound) constexpr {
    return [=](auto v) constexpr { return v > bound; };
};

inline constexpr auto GreaterOrEqual = [](auto bound) constexpr {
    return [=](auto v) constexpr { return v >= bound; };
};

inline constexpr auto LessThan = [](auto bound) constexpr {
    return [=](auto v) constexpr { return v < bound; };
};

inline constexpr auto LessOrEqual = [](auto bound) constexpr {
    return [=](auto v) constexpr { return v <= bound; };
};

inline constexpr auto EqualTo = [](auto bound) constexpr {
    return [=](auto v) constexpr { return v == bound; };
};

inline constexpr auto NotEqualTo = [](auto bound) constexpr {
    return [=](auto v) constexpr { return v != bound; };
};

// Closed interval [lo, hi]
inline constexpr auto InRange = [](auto lo, auto hi) constexpr {
    return [=](auto v) constexpr { return v >= lo && v <= hi; };
};

// Open interval (lo, hi)
inline constexpr auto InOpenRange = [](auto lo, auto hi) constexpr {
    return [=](auto v) constexpr { return v > lo && v < hi; };
};

// Half-open interval [lo, hi)
inline constexpr auto InHalfOpenRange = [](auto lo, auto hi) constexpr {
    return [=](auto v) constexpr { return v >= lo && v < hi; };
};

// Container/string predicates
inline constexpr auto NonEmpty = [](const auto& v) constexpr {
    if constexpr (requires { v.empty(); }) {
        return !v.empty();
    } else if constexpr (requires { v.size(); }) {
        return v.size() > 0;
    } else {
        static_assert(false, "Type does not support empty() or size()");
    }
};

inline constexpr auto Empty = [](const auto& v) constexpr {
    if constexpr (requires { v.empty(); }) {
        return v.empty();
    } else if constexpr (requires { v.size(); }) {
        return v.size() == 0;
    } else {
        static_assert(false, "Type does not support empty() or size()");
    }
};

inline constexpr auto SizeAtLeast = [](std::size_t min_size) constexpr {
    return [=](const auto& v) constexpr {
        return v.size() >= min_size;
    };
};

inline constexpr auto SizeAtMost = [](std::size_t max_size) constexpr {
    return [=](const auto& v) constexpr {
        return v.size() <= max_size;
    };
};

inline constexpr auto SizeExactly = [](std::size_t exact_size) constexpr {
    return [=](const auto& v) constexpr {
        return v.size() == exact_size;
    };
};

inline constexpr auto SizeInRange = [](std::size_t min_size, std::size_t max_size) constexpr {
    return [=](const auto& v) constexpr {
        return v.size() >= min_size && v.size() <= max_size;
    };
};

// Pointer predicates
inline constexpr auto NotNull = [](auto* p) constexpr {
    return p != nullptr;
};

inline constexpr auto IsNull = [](auto* p) constexpr {
    return p == nullptr;
};

// Divisibility predicates
inline constexpr auto DivisibleBy = [](auto divisor) constexpr {
    return [=](auto v) constexpr { return v % divisor == 0; };
};

inline constexpr auto Even = [](auto v) constexpr {
    return v % 2 == 0;
};

inline constexpr auto Odd = [](auto v) constexpr {
    return v % 2 != 0;
};

// Bitwise predicates
inline constexpr auto PowerOfTwo = [](auto v) constexpr {
    return v > 0 && (v & (v - 1)) == 0;
};

// Floating point predicates
inline constexpr auto Finite = [](auto v) constexpr {
    // Note: std::isfinite may not be constexpr in all implementations
    return v == v && v != v + 1; // Simple check: not NaN and not infinite
};

inline constexpr auto Normalized = [](auto v) constexpr {
    return v >= decltype(v){-1} && v <= decltype(v){1};
};

// Always true/false predicates (useful for testing)
inline constexpr auto Always = [](auto) constexpr { return true; };
inline constexpr auto Never = [](auto) constexpr { return false; };

} // namespace refine

#endif // RCPP_PREDICATES_HPP

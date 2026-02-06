// operations.hpp - Arithmetic operations with refinement preservation
// Part of the C++26 Refinement Types Library

#ifndef RCPP_OPERATIONS_HPP
#define RCPP_OPERATIONS_HPP

#include <concepts>
#include <type_traits>
#include <optional>
#include <cmath>
#include <limits>

#include "refined.hpp"
#include "predicates.hpp"

namespace refine {

// Traits for determining output refinement of operations
namespace traits {

// Marker for operations that definitely preserve a predicate.
// The T parameter restricts preservation claims to specific type categories.
// For integers, overflow can violate the predicate, so we default to false.
template<auto Pred, typename Op, typename T = void>
struct preserves {
    static constexpr bool value = false;
};

// Addition of two positive floats is positive (no overflow to negative)
template<typename T> requires std::floating_point<T>
struct preserves<Positive, std::plus<>, T> {
    static constexpr bool value = true;
};

// Multiplication of two positive floats is positive
template<typename T> requires std::floating_point<T>
struct preserves<Positive, std::multiplies<>, T> {
    static constexpr bool value = true;
};

// Addition of two non-negative floats is non-negative
template<typename T> requires std::floating_point<T>
struct preserves<NonNegative, std::plus<>, T> {
    static constexpr bool value = true;
};

// Multiplication of two non-negative floats is non-negative
template<typename T> requires std::floating_point<T>
struct preserves<NonNegative, std::multiplies<>, T> {
    static constexpr bool value = true;
};

} // namespace traits

namespace detail {

// Detect interval predicates without depending on interval.hpp
template<auto Pred>
concept has_interval_bounds = requires {
    { decltype(Pred)::lo };
    { decltype(Pred)::hi };
};

} // namespace detail

// Binary operation that attempts to preserve refinement
template<auto Pred, typename T, typename Op>
[[nodiscard]] constexpr auto refined_binop(
    const Refined<T, Pred>& lhs,
    const Refined<T, Pred>& rhs,
    Op op
) -> std::conditional_t<
    traits::preserves<Pred, Op, T>::value,
    Refined<T, Pred>,
    std::optional<Refined<T, Pred>>
> {
    auto result = op(lhs.get(), rhs.get());

    if constexpr (traits::preserves<Pred, Op, T>::value) {
        // Predicate is guaranteed to be preserved (floating-point only)
        return Refined<T, Pred>(result, assume_valid);
    } else {
        // Must check at runtime
        return try_refine<Pred>(result);
    }
}

// Arithmetic operators for refined types that return optional results
// (because we can't prove predicate preservation in general)

// Addition
template<typename T, auto Pred>
    requires (!detail::has_interval_bounds<Pred>)
[[nodiscard]] constexpr auto operator+(
    const Refined<T, Pred>& lhs,
    const Refined<T, Pred>& rhs
) -> std::conditional_t<
    traits::preserves<Pred, std::plus<>, T>::value,
    Refined<T, Pred>,
    std::optional<Refined<T, Pred>>
> {
    return refined_binop(lhs, rhs, std::plus<>{});
}

// Subtraction (rarely preserves predicates)
template<typename T, auto Pred>
    requires (!detail::has_interval_bounds<Pred>)
[[nodiscard]] constexpr std::optional<Refined<T, Pred>> operator-(
    const Refined<T, Pred>& lhs,
    const Refined<T, Pred>& rhs
) {
    return try_refine<Pred>(lhs.get() - rhs.get());
}

// Multiplication
template<typename T, auto Pred>
    requires (!detail::has_interval_bounds<Pred>)
[[nodiscard]] constexpr auto operator*(
    const Refined<T, Pred>& lhs,
    const Refined<T, Pred>& rhs
) -> std::conditional_t<
    traits::preserves<Pred, std::multiplies<>, T>::value,
    Refined<T, Pred>,
    std::optional<Refined<T, Pred>>
> {
    return refined_binop(lhs, rhs, std::multiplies<>{});
}

// Division (use with NonZero denominator)
template<typename T, auto NumPred, auto DenomPred>
[[nodiscard]] constexpr T operator/(
    const Refined<T, NumPred>& numerator,
    const Refined<T, DenomPred>& denominator
) {
    // Denominator is refined, so division is safe
    // But we lose refinement information about the result
    return numerator.get() / denominator.get();
}

// Modulo (use with NonZero divisor)
template<typename T, auto NumPred, auto DivPred>
[[nodiscard]] constexpr T operator%(
    const Refined<T, NumPred>& numerator,
    const Refined<T, DivPred>& divisor
) requires std::integral<T>
{
    return numerator.get() % divisor.get();
}

// Unary negation
template<typename T, auto Pred>
    requires (!detail::has_interval_bounds<Pred>)
[[nodiscard]] constexpr std::optional<Refined<T, Pred>> operator-(
    const Refined<T, Pred>& val
) {
    return try_refine<Pred>(-val.get());
}

// Unary plus (identity)
template<typename T, auto Pred>
[[nodiscard]] constexpr Refined<T, Pred> operator+(
    const Refined<T, Pred>& val
) {
    return val;
}

// Increment/decrement operations that return optionals
template<typename T, auto Pred>
[[nodiscard]] constexpr std::optional<Refined<T, Pred>> increment(
    const Refined<T, Pred>& val
) {
    return try_refine<Pred>(val.get() + T{1});
}

template<typename T, auto Pred>
[[nodiscard]] constexpr std::optional<Refined<T, Pred>> decrement(
    const Refined<T, Pred>& val
) {
    return try_refine<Pred>(val.get() - T{1});
}

// Safe division: requires NonZero denominator
// NOTE: For floating-point, inf/inf produces NaN. Only guards division-by-zero.
template<typename T>
[[nodiscard]] constexpr T safe_divide(
    T numerator,
    Refined<T, NonZero> denominator
) {
    return numerator / denominator.get();
}

// Safe modulo: requires NonZero divisor
template<typename T>
    requires std::integral<T>
[[nodiscard]] constexpr T safe_modulo(
    T numerator,
    Refined<T, NonZero> divisor
) {
    return numerator % divisor.get();
}

// Min/max operations preserve refinement
template<typename T, auto Pred>
[[nodiscard]] constexpr Refined<T, Pred> refined_min(
    const Refined<T, Pred>& a,
    const Refined<T, Pred>& b
) {
    return Refined<T, Pred>(
        a.get() < b.get() ? a.get() : b.get(),
        assume_valid
    );
}

template<typename T, auto Pred>
[[nodiscard]] constexpr Refined<T, Pred> refined_max(
    const Refined<T, Pred>& a,
    const Refined<T, Pred>& b
) {
    return Refined<T, Pred>(
        a.get() > b.get() ? a.get() : b.get(),
        assume_valid
    );
}

// Clamp preserves the input refinement
template<typename T, auto Pred>
[[nodiscard]] constexpr Refined<T, Pred> refined_clamp(
    const Refined<T, Pred>& val,
    const Refined<T, Pred>& lo,
    const Refined<T, Pred>& hi
) {
    const T& v = val.get();
    const T& l = lo.get();
    const T& h = hi.get();

    T result = (v < l) ? l : ((v > h) ? h : v);
    return Refined<T, Pred>(result, assume_valid);
}

// Absolute value (result is NonNegative)
template<typename T>
    requires std::floating_point<T>
[[nodiscard]] constexpr Refined<T, NonNegative> abs(T value) {
    return Refined<T, NonNegative>(
        value < T{0} ? -value : value,
        assume_valid
    );
}

template<typename T>
    requires std::signed_integral<T>
[[nodiscard]] constexpr Refined<T, NonNegative> abs(T value) {
    if (value == std::numeric_limits<T>::min()) {
        throw refinement_error(value, "abs (negation of minimum value overflows)");
    }
    return Refined<T, NonNegative>(
        value < T{0} ? -value : value,
        assume_valid
    );
}

template<typename T, auto Pred>
[[nodiscard]] constexpr Refined<T, NonNegative> abs(const Refined<T, Pred>& refined) {
    return abs(refined.get());
}

// Square (result is NonNegative for real types)
template<typename T>
    requires std::floating_point<T>
[[nodiscard]] constexpr Refined<T, NonNegative> square(T value) {
    return Refined<T, NonNegative>(value * value, assume_valid);
}

template<typename T>
    requires std::integral<T>
[[nodiscard]] constexpr Refined<T, NonNegative> square(T value) {
    // For squaring, the result is always non-negative, so we only need to
    // check against max. Use abs(value) but handle INT_MIN carefully.
    if (value != 0) {
        T abs_val = value > 0 ? value : -value;
        // If value == min (e.g. INT_MIN), -value is UB, but min*min always overflows
        // since |min| > |max| for two's complement. Detect via: if value < 0 and
        // -value would overflow (value < -max), it definitely overflows when squared.
        if (value < -std::numeric_limits<T>::max() ||
            abs_val > std::numeric_limits<T>::max() / abs_val) {
            throw refinement_error(value, "square (overflow)");
        }
    }
    return Refined<T, NonNegative>(value * value, assume_valid);
}

template<typename T, auto Pred>
[[nodiscard]] constexpr Refined<T, NonNegative> square(const Refined<T, Pred>& refined) {
    return square(refined.get());
}

// Safe sqrt for non-negative floats (preserves NonNegative)
template<typename T>
    requires std::floating_point<T>
[[nodiscard]] constexpr Refined<T, NonNegative> safe_sqrt(Refined<T, NonNegative> value) {
    return Refined<T, NonNegative>(std::sqrt(value.get()), assume_valid);
}

// Safe sqrt for positive floats (preserves Positive)
template<typename T>
    requires std::floating_point<T>
[[nodiscard]] constexpr Refined<T, Positive> safe_sqrt(Refined<T, Positive> value) {
    return Refined<T, Positive>(std::sqrt(value.get()), assume_valid);
}

// Safe log for positive floats (result can be negative, so returns plain T)
template<typename T>
    requires std::floating_point<T>
[[nodiscard]] constexpr T safe_log(Refined<T, Positive> value) {
    return std::log(value.get());
}

// Safe asin for normalized floats [-1, 1] (returns plain T)
template<typename T>
    requires std::floating_point<T>
[[nodiscard]] constexpr T safe_asin(Refined<T, Normalized> value) {
    return std::asin(value.get());
}

// Safe acos for normalized floats [-1, 1] (returns plain T)
template<typename T>
    requires std::floating_point<T>
[[nodiscard]] constexpr T safe_acos(Refined<T, Normalized> value) {
    return std::acos(value.get());
}

// Safe reciprocal for non-zero floats (returns plain T)
template<typename T>
    requires std::floating_point<T>
[[nodiscard]] constexpr T safe_reciprocal(Refined<T, NonZero> value) {
    return T{1} / value.get();
}

} // namespace refine

#endif // RCPP_OPERATIONS_HPP

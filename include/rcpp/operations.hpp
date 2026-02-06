// operations.hpp - Arithmetic operations with refinement preservation
// Part of the C++26 Refinement Types Library

#ifndef RCPP_OPERATIONS_HPP
#define RCPP_OPERATIONS_HPP

#include <concepts>
#include <type_traits>
#include <optional>

#include "refined.hpp"
#include "predicates.hpp"

namespace refine {

// Traits for determining output refinement of operations
namespace traits {

// Check if a predicate is preserved under an operation
template<auto Pred, typename T, typename Op>
concept preserves_predicate = requires(T a, T b) {
    { Op{}(a, b) } -> std::same_as<T>;
    // Note: We can't actually prove predicate preservation at compile time
    // without SMT, so this is always false by default
};

// Marker for operations that definitely preserve a predicate
template<auto Pred, typename Op>
struct preserves {
    static constexpr bool value = false;
};

// Addition of two positive numbers is positive
template<>
struct preserves<Positive, std::plus<>> {
    static constexpr bool value = true;
};

// Multiplication of two positive numbers is positive
template<>
struct preserves<Positive, std::multiplies<>> {
    static constexpr bool value = true;
};

// Addition of two non-negative numbers is non-negative
template<>
struct preserves<NonNegative, std::plus<>> {
    static constexpr bool value = true;
};

// Multiplication of two non-negative numbers is non-negative
template<>
struct preserves<NonNegative, std::multiplies<>> {
    static constexpr bool value = true;
};

} // namespace traits

// Binary operation that attempts to preserve refinement
template<auto Pred, typename T, typename Op>
[[nodiscard]] constexpr auto refined_binop(
    const Refined<T, Pred>& lhs,
    const Refined<T, Pred>& rhs,
    Op op
) -> std::conditional_t<
    traits::preserves<Pred, Op>::value,
    Refined<T, Pred>,
    std::optional<Refined<T, Pred>>
> {
    auto result = op(lhs.get(), rhs.get());

    if constexpr (traits::preserves<Pred, Op>::value) {
        // Predicate is guaranteed to be preserved
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
[[nodiscard]] constexpr auto operator+(
    const Refined<T, Pred>& lhs,
    const Refined<T, Pred>& rhs
) -> std::conditional_t<
    traits::preserves<Pred, std::plus<>>::value,
    Refined<T, Pred>,
    std::optional<Refined<T, Pred>>
> {
    return refined_binop(lhs, rhs, std::plus<>{});
}

// Subtraction (rarely preserves predicates)
template<typename T, auto Pred>
[[nodiscard]] constexpr std::optional<Refined<T, Pred>> operator-(
    const Refined<T, Pred>& lhs,
    const Refined<T, Pred>& rhs
) {
    return try_refine<Pred>(lhs.get() - rhs.get());
}

// Multiplication
template<typename T, auto Pred>
[[nodiscard]] constexpr auto operator*(
    const Refined<T, Pred>& lhs,
    const Refined<T, Pred>& rhs
) -> std::conditional_t<
    traits::preserves<Pred, std::multiplies<>>::value,
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
) requires requires { Refined<T, DenomPred>::predicate(T{1}); } // Denominator must be testable
{
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
    requires std::signed_integral<T> || std::floating_point<T>
[[nodiscard]] constexpr Refined<T, NonNegative> abs(T value) {
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
    requires std::integral<T> || std::floating_point<T>
[[nodiscard]] constexpr Refined<T, NonNegative> square(T value) {
    return Refined<T, NonNegative>(value * value, assume_valid);
}

template<typename T, auto Pred>
[[nodiscard]] constexpr Refined<T, NonNegative> square(const Refined<T, Pred>& refined) {
    return square(refined.get());
}

} // namespace refine

#endif // RCPP_OPERATIONS_HPP

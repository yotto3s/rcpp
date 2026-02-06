// interval.hpp - Interval arithmetic for range predicates
// Part of the C++26 Refinement Types Library

#ifndef RCPP_INTERVAL_HPP
#define RCPP_INTERVAL_HPP

#include "refined_type.hpp"

namespace refined {

// Structural interval predicate: closed [Lo, Hi]
// Valid as NTTP because it has no data members (bounds are template parameters).
template<auto Lo, auto Hi>
struct Interval {
    static constexpr auto lo = Lo;
    static constexpr auto hi = Hi;

    constexpr bool operator()(auto v) const { return v >= Lo && v <= Hi; }
};

// Trait to detect interval predicates
namespace traits {

template<typename T>
struct interval_traits : std::false_type {};

template<auto Lo, auto Hi>
struct interval_traits<Interval<Lo, Hi>> : std::true_type {
    static constexpr auto lo = Lo;
    static constexpr auto hi = Hi;
};

} // namespace traits

// Concept for interval predicates (takes an NTTP predicate value)
template<auto Pred>
concept interval_predicate = traits::interval_traits<decltype(Pred)>::value;

// Compile-time interval arithmetic
namespace interval_math {

namespace detail {

template<auto A, auto B>
consteval auto ct_min() {
    return A < B ? A : B;
}

template<auto A, auto B>
consteval auto ct_max() {
    return A > B ? A : B;
}

template<auto A, auto B, auto C, auto D>
consteval auto min4() {
    return ct_min<ct_min<A, B>(), ct_min<C, D>()>();
}

template<auto A, auto B, auto C, auto D>
consteval auto max4() {
    return ct_max<ct_max<A, B>(), ct_max<C, D>()>();
}

} // namespace detail

template<auto P1, auto P2>
    requires interval_predicate<P1> && interval_predicate<P2>
consteval auto add_intervals() {
    constexpr auto lo = P1.lo + P2.lo;
    constexpr auto hi = P1.hi + P2.hi;
    return Interval<lo, hi>{};
}

template<auto P1, auto P2>
    requires interval_predicate<P1> && interval_predicate<P2>
consteval auto sub_intervals() {
    constexpr auto lo = P1.lo - P2.hi;
    constexpr auto hi = P1.hi - P2.lo;
    return Interval<lo, hi>{};
}

template<auto P1, auto P2>
    requires interval_predicate<P1> && interval_predicate<P2>
consteval auto mul_intervals() {
    constexpr auto ac = P1.lo * P2.lo;
    constexpr auto ad = P1.lo * P2.hi;
    constexpr auto bc = P1.hi * P2.lo;
    constexpr auto bd = P1.hi * P2.hi;
    constexpr auto lo = detail::min4<ac, ad, bc, bd>();
    constexpr auto hi = detail::max4<ac, ad, bc, bd>();
    return Interval<lo, hi>{};
}

template<auto P>
    requires interval_predicate<P>
consteval auto negate_interval() {
    constexpr auto lo = -P.hi;
    constexpr auto hi = -P.lo;
    return Interval<lo, hi>{};
}

} // namespace interval_math

// Operator overloads for mixed-predicate interval arithmetic

// Addition: Refined<T, I1> + Refined<T, I2> -> Refined<T, I1+I2>
template<typename T, auto P1, auto P2>
    requires interval_predicate<P1> && interval_predicate<P2>
[[nodiscard]] constexpr auto operator+(
    const Refined<T, P1>& lhs,
    const Refined<T, P2>& rhs
) {
    constexpr auto result_pred = interval_math::add_intervals<P1, P2>();
    return Refined<T, result_pred>(lhs.get() + rhs.get(), assume_valid);
}

// Subtraction: Refined<T, I1> - Refined<T, I2> -> Refined<T, I1-I2>
template<typename T, auto P1, auto P2>
    requires interval_predicate<P1> && interval_predicate<P2>
[[nodiscard]] constexpr auto operator-(
    const Refined<T, P1>& lhs,
    const Refined<T, P2>& rhs
) {
    constexpr auto result_pred = interval_math::sub_intervals<P1, P2>();
    return Refined<T, result_pred>(lhs.get() - rhs.get(), assume_valid);
}

// Multiplication: Refined<T, I1> * Refined<T, I2> -> Refined<T, I1*I2>
template<typename T, auto P1, auto P2>
    requires interval_predicate<P1> && interval_predicate<P2>
[[nodiscard]] constexpr auto operator*(
    const Refined<T, P1>& lhs,
    const Refined<T, P2>& rhs
) {
    constexpr auto result_pred = interval_math::mul_intervals<P1, P2>();
    return Refined<T, result_pred>(lhs.get() * rhs.get(), assume_valid);
}

// Unary negation: -Refined<T, I> -> Refined<T, -I>
template<typename T, auto P>
    requires interval_predicate<P>
[[nodiscard]] constexpr auto operator-(
    const Refined<T, P>& val
) {
    constexpr auto result_pred = interval_math::negate_interval<P>();
    return Refined<T, result_pred>(-val.get(), assume_valid);
}

// Same-predicate overloads: these are more constrained than the generic
// same-predicate operators in operations.hpp (which have no requires clause),
// so they win overload resolution for interval types.

template<typename T, auto P>
    requires interval_predicate<P>
[[nodiscard]] constexpr auto operator+(
    const Refined<T, P>& lhs,
    const Refined<T, P>& rhs
) {
    constexpr auto result_pred = interval_math::add_intervals<P, P>();
    return Refined<T, result_pred>(lhs.get() + rhs.get(), assume_valid);
}

template<typename T, auto P>
    requires interval_predicate<P>
[[nodiscard]] constexpr auto operator-(
    const Refined<T, P>& lhs,
    const Refined<T, P>& rhs
) {
    constexpr auto result_pred = interval_math::sub_intervals<P, P>();
    return Refined<T, result_pred>(lhs.get() - rhs.get(), assume_valid);
}

template<typename T, auto P>
    requires interval_predicate<P>
[[nodiscard]] constexpr auto operator*(
    const Refined<T, P>& lhs,
    const Refined<T, P>& rhs
) {
    constexpr auto result_pred = interval_math::mul_intervals<P, P>();
    return Refined<T, result_pred>(lhs.get() * rhs.get(), assume_valid);
}

// Convenience alias
template<typename T, auto Lo, auto Hi>
using IntervalRefined = Refined<T, Interval<Lo, Hi>{}>;

} // namespace refined

#endif // RCPP_INTERVAL_HPP

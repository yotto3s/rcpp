// interval.hpp - Interval arithmetic for range predicates
// Part of the C++26 Refinement Types Library

#ifndef REFINERY_INTERVAL_HPP
#define REFINERY_INTERVAL_HPP

#include <concepts>
#include <limits>

#include "refined_type.hpp"

namespace refinery {

// Structural interval predicate: closed [Lo, Hi]
// Valid as NTTP because it has no data members (bounds are template
// parameters).
template <auto Lo, auto Hi> struct Interval {
    static constexpr auto lo = Lo;
    static constexpr auto hi = Hi;

    constexpr bool operator()(auto v) const { return v >= Lo && v <= Hi; }
};

// Trait to detect interval predicates
namespace traits {

template <typename T> struct interval_traits : std::false_type {};

template <auto Lo, auto Hi>
struct interval_traits<Interval<Lo, Hi>> : std::true_type {
    static constexpr auto lo = Lo;
    static constexpr auto hi = Hi;
};

} // namespace traits

// Concept for interval predicates (takes an NTTP predicate value)
template <auto Pred>
concept interval_predicate = traits::interval_traits<decltype(Pred)>::value;

// Compile-time interval arithmetic
namespace interval_math {

namespace detail {

template <auto A, auto B> consteval auto ct_min() { return A < B ? A : B; }

template <auto A, auto B> consteval auto ct_max() { return A > B ? A : B; }

template <auto A, auto B, auto C, auto D> consteval auto min4() {
    return ct_min<ct_min<A, B>(), ct_min<C, D>()>();
}

template <auto A, auto B, auto C, auto D> consteval auto max4() {
    return ct_max<ct_max<A, B>(), ct_max<C, D>()>();
}

// Saturating arithmetic for compile-time interval bound computation.
// Clamps to numeric limits instead of overflowing.
template <typename T>
consteval T sat_add(T a, T b) {
    if constexpr (std::integral<T>) {
        if (b > 0 && a > std::numeric_limits<T>::max() - b)
            return std::numeric_limits<T>::max();
        if (b < 0 && a < std::numeric_limits<T>::min() - b)
            return std::numeric_limits<T>::min();
    } else if constexpr (std::floating_point<T>) {
        constexpr auto mx = std::numeric_limits<T>::max();
        if ((a > 0 && b > 0) && (a > mx - b)) return mx;
        if ((a < 0 && b < 0) && (a < -mx - b)) return -mx;
    }
    return a + b;
}

template <typename T>
consteval T sat_sub(T a, T b) {
    if constexpr (std::integral<T>) {
        if (b < 0 && a > std::numeric_limits<T>::max() + b)
            return std::numeric_limits<T>::max();
        if (b > 0 && a < std::numeric_limits<T>::min() + b)
            return std::numeric_limits<T>::min();
    } else if constexpr (std::floating_point<T>) {
        constexpr auto mx = std::numeric_limits<T>::max();
        if ((a > 0 && b < 0) && (a > mx + b)) return mx;
        if ((a < 0 && b > 0) && (a < -mx + b)) return -mx;
    }
    return a - b;
}

template <typename T>
consteval T sat_mul(T a, T b) {
    if (a == T{0} || b == T{0}) return T{0};
    if constexpr (std::integral<T>) {
        if (a > 0) {
            if (b > 0) {
                if (a > std::numeric_limits<T>::max() / b)
                    return std::numeric_limits<T>::max();
            } else {
                if (b < std::numeric_limits<T>::min() / a)
                    return std::numeric_limits<T>::min();
            }
        } else {
            if (b > 0) {
                if (a < std::numeric_limits<T>::min() / b)
                    return std::numeric_limits<T>::min();
            } else {
                if (a < std::numeric_limits<T>::max() / b)
                    return std::numeric_limits<T>::max();
            }
        }
    } else if constexpr (std::floating_point<T>) {
        constexpr auto mx = std::numeric_limits<T>::max();
        auto abs_a = a < T{0} ? -a : a;
        auto abs_b = b < T{0} ? -b : b;
        if (abs_a > mx / abs_b) {
            bool neg = (a < T{0}) != (b < T{0});
            return neg ? -mx : mx;
        }
    }
    return a * b;
}

template <typename T>
consteval T sat_neg(T a) {
    if constexpr (std::integral<T>) {
        if (a == std::numeric_limits<T>::min())
            return std::numeric_limits<T>::max();
    }
    return -a;
}

} // namespace detail

template <auto P1, auto P2>
    requires interval_predicate<P1> && interval_predicate<P2>
consteval auto add_intervals() {
    using T = decltype(P1.lo);
    constexpr auto lo = detail::sat_add<T>(P1.lo, P2.lo);
    constexpr auto hi = detail::sat_add<T>(P1.hi, P2.hi);
    return Interval<lo, hi>{};
}

template <auto P1, auto P2>
    requires interval_predicate<P1> && interval_predicate<P2>
consteval auto sub_intervals() {
    using T = decltype(P1.lo);
    constexpr auto lo = detail::sat_sub<T>(P1.lo, P2.hi);
    constexpr auto hi = detail::sat_sub<T>(P1.hi, P2.lo);
    return Interval<lo, hi>{};
}

template <auto P1, auto P2>
    requires interval_predicate<P1> && interval_predicate<P2>
consteval auto mul_intervals() {
    using T = decltype(P1.lo);
    constexpr auto ac = detail::sat_mul<T>(P1.lo, P2.lo);
    constexpr auto ad = detail::sat_mul<T>(P1.lo, P2.hi);
    constexpr auto bc = detail::sat_mul<T>(P1.hi, P2.lo);
    constexpr auto bd = detail::sat_mul<T>(P1.hi, P2.hi);
    constexpr auto lo = detail::min4<ac, ad, bc, bd>();
    constexpr auto hi = detail::max4<ac, ad, bc, bd>();
    return Interval<lo, hi>{};
}

template <auto P>
    requires interval_predicate<P>
consteval auto negate_interval() {
    using T = decltype(P.lo);
    constexpr auto lo = detail::sat_neg<T>(P.hi);
    constexpr auto hi = detail::sat_neg<T>(P.lo);
    return Interval<lo, hi>{};
}

} // namespace interval_math

// Checked integer arithmetic — throws refinement_error on overflow
namespace detail {

template <typename T>
    requires std::integral<T>
constexpr T checked_add(T a, T b) {
    if (b > 0 && a > std::numeric_limits<T>::max() - b)
        throw refinement_error("integer overflow in addition");
    if (b < 0 && a < std::numeric_limits<T>::min() - b)
        throw refinement_error("integer underflow in addition");
    return a + b;
}

template <typename T>
    requires std::integral<T>
constexpr T checked_sub(T a, T b) {
    if (b < 0 && a > std::numeric_limits<T>::max() + b)
        throw refinement_error("integer overflow in subtraction");
    if (b > 0 && a < std::numeric_limits<T>::min() + b)
        throw refinement_error("integer underflow in subtraction");
    return a - b;
}

template <typename T>
    requires std::integral<T>
constexpr T checked_mul(T a, T b) {
    if (a == 0 || b == 0) return T{0};
    if (a > 0) {
        if (b > 0) {
            if (a > std::numeric_limits<T>::max() / b)
                throw refinement_error("integer overflow in multiplication");
        } else {
            if (b < std::numeric_limits<T>::min() / a)
                throw refinement_error("integer underflow in multiplication");
        }
    } else {
        if (b > 0) {
            if (a < std::numeric_limits<T>::min() / b)
                throw refinement_error("integer underflow in multiplication");
        } else {
            if (a < std::numeric_limits<T>::max() / b)
                throw refinement_error("integer overflow in multiplication");
        }
    }
    return a * b;
}

} // namespace detail

// Operator overloads for mixed-predicate interval arithmetic

// Addition: Refined<T, I1> + Refined<T, I2> -> Refined<T, I1+I2>
template <typename T, auto P1, auto P2>
    requires interval_predicate<P1> && interval_predicate<P2>
[[nodiscard]] constexpr auto operator+(const Refined<T, P1>& lhs,
                                       const Refined<T, P2>& rhs) {
    constexpr auto result_pred = interval_math::add_intervals<P1, P2>();
    if constexpr (std::integral<T>) {
        return Refined<T, result_pred>(detail::checked_add(lhs.get(), rhs.get()), assume_valid);
    } else {
        return Refined<T, result_pred>(lhs.get() + rhs.get(), assume_valid);
    }
}

// Subtraction: Refined<T, I1> - Refined<T, I2> -> Refined<T, I1-I2>
template <typename T, auto P1, auto P2>
    requires interval_predicate<P1> && interval_predicate<P2>
[[nodiscard]] constexpr auto operator-(const Refined<T, P1>& lhs,
                                       const Refined<T, P2>& rhs) {
    constexpr auto result_pred = interval_math::sub_intervals<P1, P2>();
    if constexpr (std::integral<T>) {
        return Refined<T, result_pred>(detail::checked_sub(lhs.get(), rhs.get()), assume_valid);
    } else {
        return Refined<T, result_pred>(lhs.get() - rhs.get(), assume_valid);
    }
}

// Multiplication: Refined<T, I1> * Refined<T, I2> -> Refined<T, I1*I2>
template <typename T, auto P1, auto P2>
    requires interval_predicate<P1> && interval_predicate<P2>
[[nodiscard]] constexpr auto operator*(const Refined<T, P1>& lhs,
                                       const Refined<T, P2>& rhs) {
    constexpr auto result_pred = interval_math::mul_intervals<P1, P2>();
    if constexpr (std::integral<T>) {
        return Refined<T, result_pred>(detail::checked_mul(lhs.get(), rhs.get()), assume_valid);
    } else {
        return Refined<T, result_pred>(lhs.get() * rhs.get(), assume_valid);
    }
}

// Unary negation: -Refined<T, I> -> Refined<T, -I>
template <typename T, auto P>
    requires interval_predicate<P>
[[nodiscard]] constexpr auto operator-(const Refined<T, P>& val) {
    constexpr auto result_pred = interval_math::negate_interval<P>();
    if constexpr (std::integral<T>) {
        return Refined<T, result_pred>(detail::checked_sub(T{0}, val.get()), assume_valid);
    } else {
        return Refined<T, result_pred>(-val.get(), assume_valid);
    }
}

// Same-predicate overloads: these are more constrained than the generic
// same-predicate operators in operations.hpp (which have no requires clause),
// so they win overload resolution for interval types.

template <typename T, auto P>
    requires interval_predicate<P>
[[nodiscard]] constexpr auto operator+(const Refined<T, P>& lhs,
                                       const Refined<T, P>& rhs) {
    constexpr auto result_pred = interval_math::add_intervals<P, P>();
    if constexpr (std::integral<T>) {
        return Refined<T, result_pred>(detail::checked_add(lhs.get(), rhs.get()), assume_valid);
    } else {
        return Refined<T, result_pred>(lhs.get() + rhs.get(), assume_valid);
    }
}

template <typename T, auto P>
    requires interval_predicate<P>
[[nodiscard]] constexpr auto operator-(const Refined<T, P>& lhs,
                                       const Refined<T, P>& rhs) {
    constexpr auto result_pred = interval_math::sub_intervals<P, P>();
    if constexpr (std::integral<T>) {
        return Refined<T, result_pred>(detail::checked_sub(lhs.get(), rhs.get()), assume_valid);
    } else {
        return Refined<T, result_pred>(lhs.get() - rhs.get(), assume_valid);
    }
}

template <typename T, auto P>
    requires interval_predicate<P>
[[nodiscard]] constexpr auto operator*(const Refined<T, P>& lhs,
                                       const Refined<T, P>& rhs) {
    constexpr auto result_pred = interval_math::mul_intervals<P, P>();
    if constexpr (std::integral<T>) {
        return Refined<T, result_pred>(detail::checked_mul(lhs.get(), rhs.get()), assume_valid);
    } else {
        return Refined<T, result_pred>(lhs.get() * rhs.get(), assume_valid);
    }
}

// Convenience alias
template <typename T, auto Lo, auto Hi>
using IntervalRefined = Refined<T, Interval<Lo, Hi>{}>;

} // namespace refinery

#endif // REFINERY_INTERVAL_HPP

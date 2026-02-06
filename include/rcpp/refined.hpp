// refined.hpp - Core Refined<T, Predicate> wrapper type
// Part of the C++26 Refinement Types Library

#ifndef RCPP_REFINED_HPP
#define RCPP_REFINED_HPP

#include <concepts>
#include <format>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>
#include <stdexcept>

#include "diagnostics.hpp"

namespace refine {

// Concept to check if a predicate is valid for a type
template<typename Pred, typename T>
concept predicate_for = requires(Pred pred, T value) {
    { pred(value) } -> std::convertible_to<bool>;
};

// Concept for predicates that can be evaluated at compile time
template<typename Pred, typename T>
concept constexpr_predicate_for = predicate_for<Pred, T> &&
    requires(Pred pred, T value) {
        { std::bool_constant<(pred(T{}), true)>{} };
    };

// Core refinement type wrapper
template<typename T, auto Predicate>
    requires predicate_for<decltype(Predicate), T>
class Refined {
public:
    using value_type = T;
    static constexpr auto predicate = Predicate;

private:
    T value_;

    // Helper to build error message
    static consteval std::string build_error_message(const T& value) {
        using namespace std::meta;
        std::string msg = "Refinement violation: ";
        msg += display_string_of(reflect_constant(value));
        msg += " does not satisfy predicate";
        return msg;
    }

public:
    // Compile-time verified construction (consteval)
    // This will fail at compile time if the predicate is not satisfied
    consteval explicit Refined(T value) : value_(std::move(value)) {
        if (!Predicate(value_)) {
            // Use std::meta::exception for rich compile-time errors
            // Note: We use ^^Refined as the 'from' parameter since we can't
            // reflect the non-type template parameter directly
            throw std::meta::exception(
                build_error_message(value_),
                ^^Refined
            );
        }
    }

    // Runtime checked construction
    // Throws refinement_error if predicate is not satisfied
    constexpr explicit Refined(T value, runtime_check_t) : value_(std::move(value)) {
        if (!Predicate(value_)) {
            throw refinement_error(value_);
        }
    }

    // Unchecked construction (for trusted contexts)
    // WARNING: Caller is responsible for ensuring predicate holds
    constexpr explicit Refined(T value, assume_valid_t) noexcept
        : value_(std::move(value))
    {}

    // Default constructor only if T is default constructible AND default satisfies predicate
    consteval Refined() requires std::default_initializable<T>
        : Refined(T{})
    {}

    // Copy and move constructors
    constexpr Refined(const Refined&) = default;
    constexpr Refined(Refined&&) = default;

    // Copy and move assignment
    constexpr Refined& operator=(const Refined&) = default;
    constexpr Refined& operator=(Refined&&) = default;

    // Access the underlying value
    [[nodiscard]] constexpr const T& get() const noexcept { return value_; }
    [[nodiscard]] constexpr const T& operator*() const noexcept { return value_; }
    [[nodiscard]] constexpr const T* operator->() const noexcept { return &value_; }

    // Implicit conversion to const reference of underlying type
    [[nodiscard]] constexpr operator const T&() const noexcept { return value_; }

    // Explicit conversion to value (allows modification outside the wrapper)
    [[nodiscard]] constexpr T release() && noexcept { return std::move(value_); }

    // Check if a value would satisfy the predicate
    [[nodiscard]] static constexpr bool is_valid(const T& value) noexcept {
        return Predicate(value);
    }

    // Equality comparison
    [[nodiscard]] friend constexpr bool operator==(const Refined& lhs, const Refined& rhs) {
        return lhs.value_ == rhs.value_;
    }

    [[nodiscard]] friend constexpr bool operator==(const Refined& lhs, const T& rhs) {
        return lhs.value_ == rhs;
    }

    // Three-way comparison
    [[nodiscard]] friend constexpr auto operator<=>(const Refined& lhs, const Refined& rhs)
        requires std::three_way_comparable<T>
    {
        return lhs.value_ <=> rhs.value_;
    }

    [[nodiscard]] friend constexpr auto operator<=>(const Refined& lhs, const T& rhs)
        requires std::three_way_comparable<T>
    {
        return lhs.value_ <=> rhs;
    }
};

// Factory function for compile-time construction
template<auto Predicate, typename T>
    requires predicate_for<decltype(Predicate), T>
[[nodiscard]] consteval auto make_refined(T value) {
    return Refined<T, Predicate>(std::move(value));
}

// Factory function for runtime checked construction
template<auto Predicate, typename T>
    requires predicate_for<decltype(Predicate), T>
[[nodiscard]] constexpr auto make_refined_checked(T value) {
    return Refined<T, Predicate>(std::move(value), runtime_check);
}

// Try to create a refined value, returning optional
template<typename RefinedT, typename T = typename RefinedT::value_type>
[[nodiscard]] constexpr std::optional<RefinedT> try_refine(T value) noexcept {
    if (RefinedT::predicate(value)) {
        return RefinedT(std::move(value), assume_valid);
    }
    return std::nullopt;
}

// Try to create a refined value with explicit predicate
template<auto Predicate, typename T>
    requires predicate_for<decltype(Predicate), T>
[[nodiscard]] constexpr std::optional<Refined<T, Predicate>> try_refine(T value) noexcept {
    if (Predicate(value)) {
        return Refined<T, Predicate>(std::move(value), assume_valid);
    }
    return std::nullopt;
}

// Assume a value is refined (unchecked, use with caution)
template<auto Predicate, typename T>
    requires predicate_for<decltype(Predicate), T>
[[nodiscard]] constexpr auto assume_refined(T value) noexcept {
    return Refined<T, Predicate>(std::move(value), assume_valid);
}

// Coerce from one refinement to another (runtime checked)
template<typename ToRefined, typename FromRefined>
    requires std::same_as<typename ToRefined::value_type, typename FromRefined::value_type>
[[nodiscard]] constexpr ToRefined refine_to(const FromRefined& from) {
    return ToRefined(from.get(), runtime_check);
}

// Try to coerce from one refinement to another
template<typename ToRefined, typename FromRefined>
    requires std::same_as<typename ToRefined::value_type, typename FromRefined::value_type>
[[nodiscard]] constexpr std::optional<ToRefined> try_refine_to(const FromRefined& from) noexcept {
    return try_refine<ToRefined>(from.get());
}

// Transform a refined value, producing a new refined value
template<auto NewPredicate, typename T, auto OldPredicate, typename F>
    requires std::invocable<F, const T&> &&
             predicate_for<decltype(NewPredicate), std::invoke_result_t<F, const T&>>
[[nodiscard]] constexpr auto transform_refined(
    const Refined<T, OldPredicate>& refined,
    F&& func
) {
    using ResultT = std::invoke_result_t<F, const T&>;
    return Refined<ResultT, NewPredicate>(
        std::invoke(std::forward<F>(func), refined.get()),
        runtime_check
    );
}

// Check if two refined types have the same predicate
template<typename R1, typename R2>
concept same_predicate = requires {
    requires std::same_as<typename R1::value_type, typename R2::value_type>;
    requires R1::predicate == R2::predicate;
};

// Concept for refined types
template<typename T>
concept is_refined = requires {
    typename T::value_type;
    { T::predicate } -> std::convertible_to<bool(typename T::value_type)>;
};

// Get reflection info for the Refined type itself
template<typename T, auto Predicate>
consteval std::meta::info type_info(const Refined<T, Predicate>&) {
    return ^^Refined<T, Predicate>;
}

} // namespace refine

// Formatter specialization for Refined types
template<typename T, auto Pred>
struct std::formatter<refine::Refined<T, Pred>> : std::formatter<T> {
    template<typename FormatContext>
    auto format(const refine::Refined<T, Pred>& refined, FormatContext& ctx) const {
        return std::formatter<T>::format(refined.get(), ctx);
    }
};

#endif // RCPP_REFINED_HPP

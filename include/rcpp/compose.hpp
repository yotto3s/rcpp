// compose.hpp - Predicate composition utilities
// Part of the C++26 Refinement Types Library

#ifndef RCPP_COMPOSE_HPP
#define RCPP_COMPOSE_HPP

#include <concepts>
#include <type_traits>
#include <utility>
#include <functional>
#include <vector>
#include <cstddef>

namespace refine {

// Conjunction of predicates: All<P1, P2, ...>
// Value must satisfy ALL predicates
template<auto... Preds>
inline constexpr auto All = [](const auto& v) constexpr {
    return (Preds(v) && ...);
};

// Disjunction of predicates: Any<P1, P2, ...>
// Value must satisfy AT LEAST ONE predicate
template<auto... Preds>
inline constexpr auto Any = [](const auto& v) constexpr {
    return (Preds(v) || ...);
};

// Negation of a predicate: Not<P>
template<auto Pred>
inline constexpr auto Not = [](const auto& v) constexpr {
    return !Pred(v);
};

// Implication: If<P1, P2> means (P1 => P2), i.e., (!P1 || P2)
// If P1 holds, then P2 must also hold
template<auto Condition, auto Consequence>
inline constexpr auto If = [](const auto& v) constexpr {
    return !Condition(v) || Consequence(v);
};

// Biconditional: Iff<P1, P2> means (P1 <=> P2)
// Both predicates must have the same truth value
template<auto P1, auto P2>
inline constexpr auto Iff = [](const auto& v) constexpr {
    return P1(v) == P2(v);
};

// Exclusive or: Xor<P1, P2>
// Exactly one of the predicates must be satisfied
template<auto P1, auto P2>
inline constexpr auto Xor = [](const auto& v) constexpr {
    return P1(v) != P2(v);
};

// Exactly N predicates must be satisfied
template<std::size_t N, auto... Preds>
inline constexpr auto ExactlyN = [](const auto& v) constexpr {
    std::size_t count = 0;
    ((count += Preds(v) ? 1 : 0), ...);
    return count == N;
};

// At least N predicates must be satisfied
template<std::size_t N, auto... Preds>
inline constexpr auto AtLeastN = [](const auto& v) constexpr {
    std::size_t count = 0;
    ((count += Preds(v) ? 1 : 0), ...);
    return count >= N;
};

// At most N predicates must be satisfied
template<std::size_t N, auto... Preds>
inline constexpr auto AtMostN = [](const auto& v) constexpr {
    std::size_t count = 0;
    ((count += Preds(v) ? 1 : 0), ...);
    return count <= N;
};

// Runtime predicate composition (for dynamic predicates)
namespace runtime {

template<typename T>
struct AllOf {
    std::vector<std::function<bool(const T&)>> predicates;

    template<typename... Preds>
    explicit AllOf(Preds... preds) : predicates{preds...} {}

    bool operator()(const T& v) const {
        for (const auto& pred : predicates) {
            if (!pred(v)) return false;
        }
        return true;
    }
};

template<typename T>
struct AnyOf {
    std::vector<std::function<bool(const T&)>> predicates;

    template<typename... Preds>
    explicit AnyOf(Preds... preds) : predicates{preds...} {}

    bool operator()(const T& v) const {
        for (const auto& pred : predicates) {
            if (pred(v)) return true;
        }
        return false;
    }
};

template<typename T>
struct NoneOf {
    std::vector<std::function<bool(const T&)>> predicates;

    template<typename... Preds>
    explicit NoneOf(Preds... preds) : predicates{preds...} {}

    bool operator()(const T& v) const {
        for (const auto& pred : predicates) {
            if (pred(v)) return false;
        }
        return true;
    }
};

} // namespace runtime

// Predicate on a member/projection
// Apply<Proj, Pred> checks Pred(Proj(v))
template<auto Projection, auto Pred>
inline constexpr auto Apply = [](const auto& v) constexpr {
    return Pred(Projection(v));
};

// Helper for member access
template<auto MemPtr, auto Pred>
inline constexpr auto OnMember = [](const auto& v) constexpr {
    return Pred(v.*MemPtr);
};

} // namespace refine

#endif // RCPP_COMPOSE_HPP

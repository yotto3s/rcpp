// refined_container.hpp - Size-predicated container wrappers
// Part of the C++26 Refinement Types Library

#ifndef REFINERY_REFINED_CONTAINER_HPP
#define REFINERY_REFINED_CONTAINER_HPP

#include <cstddef>
#include <limits>
#include <type_traits>

#include "refined_type.hpp"

namespace refinery {

// Structural size predicate: closed [Lo, Hi] on container size
// Analogous to Interval<Lo, Hi> for numeric values.
template <std::size_t Lo,
          std::size_t Hi = std::numeric_limits<std::size_t>::max()>
struct SizeInterval {
    static constexpr std::size_t lo = Lo;
    static constexpr std::size_t hi = Hi;

    constexpr bool operator()(auto s) const {
        return static_cast<std::size_t>(s) >= Lo &&
               static_cast<std::size_t>(s) <= Hi;
    }
};

// Trait to detect SizeInterval predicates
namespace traits {

template <typename T> struct size_interval_traits : std::false_type {};

template <std::size_t Lo, std::size_t Hi>
struct size_interval_traits<SizeInterval<Lo, Hi>> : std::true_type {
    static constexpr std::size_t lo = Lo;
    static constexpr std::size_t hi = Hi;
};

} // namespace traits

// Concept for SizeInterval predicates (takes an NTTP predicate value)
template <auto Pred>
concept size_interval_predicate =
    traits::size_interval_traits<decltype(Pred)>::value;

} // namespace refinery

#endif // REFINERY_REFINED_CONTAINER_HPP

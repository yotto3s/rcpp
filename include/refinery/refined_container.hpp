// refined_container.hpp - Size-predicated container wrappers
// Part of the C++26 Refinement Types Library

#ifndef REFINERY_REFINED_CONTAINER_HPP
#define REFINERY_REFINED_CONTAINER_HPP

#include <cstddef>
#include <concepts>
#include <format>
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

// Concept: type has a .size() method returning something convertible to size_t
template <typename C>
concept SizedContainer = requires(const C& c) {
    { c.size() } -> std::convertible_to<std::size_t>;
};

// Core refined container wrapper
template <SizedContainer Container, auto SizePredicate>
    requires predicate_for<decltype(SizePredicate), std::size_t>
class RefinedContainer {
  public:
    using container_type = Container;
    using value_type = typename Container::value_type;
    static constexpr auto size_predicate = SizePredicate;

  private:
    Container container_;

  public:
    // Runtime checked construction
    constexpr explicit RefinedContainer(Container container, runtime_check_t)
        : container_(std::move(container)) {
        if (!SizePredicate(container_.size())) {
            throw refinement_error(
                std::format("Size refinement violation: size {} does not "
                            "satisfy predicate",
                            container_.size()));
        }
    }

    // Unchecked construction (caller guarantees predicate holds)
    constexpr explicit RefinedContainer(Container container,
                                        assume_valid_t) noexcept
        : container_(std::move(container)) {}

    // Copy and move
    constexpr RefinedContainer(const RefinedContainer&) = default;
    constexpr RefinedContainer(RefinedContainer&&) = default;
    constexpr RefinedContainer& operator=(const RefinedContainer&) = default;
    constexpr RefinedContainer& operator=(RefinedContainer&&) = default;

    // Access the underlying container
    [[nodiscard]] constexpr const Container& get() const noexcept {
        return container_;
    }

    // Release ownership of the underlying container
    [[nodiscard]] constexpr Container release() && noexcept {
        return std::move(container_);
    }

    // Size and emptiness (always available)
    [[nodiscard]] constexpr std::size_t size() const noexcept {
        return container_.size();
    }

    [[nodiscard]] constexpr bool empty() const noexcept {
        return container_.empty();
    }

    // Iterators (always available)
    [[nodiscard]] constexpr auto begin() const noexcept {
        return container_.begin();
    }

    [[nodiscard]] constexpr auto end() const noexcept {
        return container_.end();
    }

    [[nodiscard]] constexpr auto cbegin() const noexcept {
        return container_.cbegin();
    }

    [[nodiscard]] constexpr auto cend() const noexcept {
        return container_.cend();
    }

    // Data pointer (for contiguous containers)
    [[nodiscard]] constexpr auto data() const noexcept
        requires requires(const Container& c) { c.data(); }
    {
        return container_.data();
    }

    // Predicate-gated access: only available when size >= 1 is provable
    [[nodiscard]] constexpr const auto& front() const
        requires(size_interval_predicate<SizePredicate> &&
                 traits::size_interval_traits<
                     decltype(SizePredicate)>::lo >= 1)
    {
        return container_.front();
    }

    [[nodiscard]] constexpr const auto& back() const
        requires(size_interval_predicate<SizePredicate> &&
                 traits::size_interval_traits<
                     decltype(SizePredicate)>::lo >= 1)
    {
        return container_.back();
    }
};

} // namespace refinery

#endif // REFINERY_REFINED_CONTAINER_HPP

// refined_container.hpp - Size-predicated container wrappers
// Part of the C++26 Refinement Types Library

#ifndef REFINERY_REFINED_CONTAINER_HPP
#define REFINERY_REFINED_CONTAINER_HPP

#include <array>
#include <cstddef>
#include <concepts>
#include <format>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

#include <meta>

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

// Compute new SizeInterval after shifting bounds by Delta.
// Uses saturating arithmetic: clamps to 0 on underflow, SIZE_MAX on overflow.
template <auto Pred, std::ptrdiff_t Delta>
    requires size_interval_predicate<Pred>
consteval auto size_interval_shift() {
    constexpr auto lo = traits::size_interval_traits<decltype(Pred)>::lo;
    constexpr auto hi = traits::size_interval_traits<decltype(Pred)>::hi;
    constexpr auto max_sz = std::numeric_limits<std::size_t>::max();

    constexpr auto new_lo = []{
        if constexpr (Delta >= 0) {
            constexpr auto d = static_cast<std::size_t>(Delta);
            return (lo > max_sz - d) ? max_sz : lo + d;
        } else {
            constexpr auto d = static_cast<std::size_t>(-Delta);
            return (lo < d) ? std::size_t{0} : lo - d;
        }
    }();

    constexpr auto new_hi = []{
        if constexpr (Delta >= 0) {
            constexpr auto d = static_cast<std::size_t>(Delta);
            return (hi > max_sz - d) ? max_sz : hi + d;
        } else {
            constexpr auto d = static_cast<std::size_t>(-Delta);
            return (hi < d) ? std::size_t{0} : hi - d;
        }
    }();

    return SizeInterval<new_lo, new_hi>{};
}

// Concept: type has a .size() method returning something convertible to size_t
template <typename C>
concept SizedContainer = requires(const C& c) {
    { c.size() } -> std::convertible_to<std::size_t>;
};

// Branded index -- only constructible by its matching SizeGuard
template <auto Tag>
class GuardedIndex {
  public:
    [[nodiscard]] constexpr std::size_t get() const noexcept { return index_; }

  private:
    std::size_t index_;

    constexpr explicit GuardedIndex(std::size_t idx) noexcept : index_(idx) {}

    template <auto T>
    friend class SizeGuard;
};

// Size witness -- captures runtime size and produces branded indices
template <auto Tag>
class SizeGuard {
  public:
    constexpr explicit SizeGuard(std::size_t size) noexcept : size_(size) {}

    [[nodiscard]] constexpr std::optional<GuardedIndex<Tag>>
    check(std::size_t idx) const noexcept {
        if (idx < size_) {
            return GuardedIndex<Tag>(idx);
        }
        return std::nullopt;
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept {
        return size_;
    }

  private:
    std::size_t size_;
};

// Frozen container -- accepts only matching GuardedIndex for element access
template <SizedContainer Container, auto SizePredicate, auto Tag>
class FrozenContainer {
  public:
    constexpr explicit FrozenContainer(Container container,
                                       assume_valid_t) noexcept
        : container_(std::move(container)) {}

    [[nodiscard]] constexpr const auto&
    operator[](GuardedIndex<Tag> idx) const {
        return container_[idx.get()];
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept {
        return container_.size();
    }

    [[nodiscard]] constexpr bool empty() const noexcept {
        return container_.empty();
    }

    [[nodiscard]] constexpr auto begin() const noexcept {
        return container_.begin();
    }

    [[nodiscard]] constexpr auto end() const noexcept {
        return container_.end();
    }

  private:
    Container container_;
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
    // Compile-time verified construction (consteval)
    // Fails at compile time if the size predicate is not satisfied.
    consteval explicit RefinedContainer(Container container)
        : container_(std::move(container)) {
        if (!SizePredicate(container_.size())) {
            throw std::meta::exception(
                std::format("Size refinement violation: size {} does not "
                            "satisfy predicate",
                            container_.size()),
                ^^RefinedContainer);
        }
    }

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

    // Static indexing: accepts Refined<size_t, IndexPred> where the index
    // upper bound is provably less than the container's size lower bound.
    template <auto IndexPred>
        requires(size_interval_predicate<SizePredicate> &&
                 detail::has_interval_bounds<IndexPred> &&
                 decltype(IndexPred)::hi <
                     traits::size_interval_traits<
                         decltype(SizePredicate)>::lo)
    [[nodiscard]] constexpr const auto&
    operator[](Refined<std::size_t, IndexPred> idx) const {
        return container_[idx.get()];
    }

    // --- Mutations (consume *this, return new RefinedContainer) ---

    // push_back: delta +1
    template <typename V>
    [[nodiscard]] constexpr auto push_back(V&& value) &&
        requires requires(Container& c, V&& v) {
            c.push_back(std::forward<V>(v));
        }
    {
        container_.push_back(std::forward<V>(value));
        constexpr auto new_pred = size_interval_shift<SizePredicate, 1>();
        return RefinedContainer<Container, new_pred>(std::move(container_),
                                                     assume_valid);
    }

    // pop_back: delta -1, requires non-empty
    [[nodiscard]] constexpr auto pop_back() &&
        requires requires(Container& c) { c.pop_back(); } &&
                 (size_interval_predicate<SizePredicate> &&
                  traits::size_interval_traits<
                      decltype(SizePredicate)>::lo >= 1)
    {
        container_.pop_back();
        constexpr auto new_pred = size_interval_shift<SizePredicate, -1>();
        return RefinedContainer<Container, new_pred>(std::move(container_),
                                                     assume_valid);
    }

    // emplace_back: delta +1
    template <typename... Args>
    [[nodiscard]] constexpr auto emplace_back(Args&&... args) &&
        requires requires(Container& c, Args&&... a) {
            c.emplace_back(std::forward<Args>(a)...);
        }
    {
        container_.emplace_back(std::forward<Args>(args)...);
        constexpr auto new_pred = size_interval_shift<SizePredicate, 1>();
        return RefinedContainer<Container, new_pred>(std::move(container_),
                                                     assume_valid);
    }

    // Append from std::array (compile-time-known size N)
    template <typename V, std::size_t N>
    [[nodiscard]] constexpr auto append(const std::array<V, N>& source) &&
        requires requires(Container& c, const V& v) {
            c.push_back(v);
        }
    {
        for (const auto& elem : source) {
            container_.push_back(elem);
        }
        constexpr auto new_pred =
            size_interval_shift<SizePredicate,
                                static_cast<std::ptrdiff_t>(N)>();
        return RefinedContainer<Container, new_pred>(std::move(container_),
                                                     assume_valid);
    }

    // Append from another RefinedContainer (uses source's lower bound)
    template <SizedContainer C2, auto SizePred2>
    [[nodiscard]] constexpr auto
    append(RefinedContainer<C2, SizePred2>&& source) &&
        requires requires(Container& c,
                          const typename C2::value_type& v) {
            c.push_back(v);
        } && size_interval_predicate<SizePred2>
    {
        auto released = std::move(source).release();
        for (auto& elem : released) {
            container_.push_back(std::move(elem));
        }
        constexpr std::ptrdiff_t delta = static_cast<std::ptrdiff_t>(
            traits::size_interval_traits<decltype(SizePred2)>::lo);
        constexpr auto new_pred =
            size_interval_shift<SizePredicate, delta>();
        return RefinedContainer<Container, new_pred>(std::move(container_),
                                                     assume_valid);
    }

    // Freeze: capture actual size and produce branded guard + frozen container.
    // Each call site gets a unique Tag type via the default lambda NTTP.
    template <auto Tag = []{}>
    [[nodiscard]] constexpr auto freeze() && {
        auto sz = container_.size();
        return std::pair{
            SizeGuard<Tag>{sz},
            FrozenContainer<Container, SizePredicate, Tag>(
                std::move(container_), assume_valid)};
    }
};

// Convenience alias: RefinedContainer with SizeInterval<Lo, Hi>
template <SizedContainer C, std::size_t Lo,
          std::size_t Hi = std::numeric_limits<std::size_t>::max()>
using SizeRefined = RefinedContainer<C, SizeInterval<Lo, Hi>{}>;

// Convenience alias: non-empty container (size >= 1)
template <SizedContainer C>
using NonEmptyContainer = RefinedContainer<C, SizeInterval<1>{}>;

// Zero-overhead guarantee: RefinedContainer is the same size as Container
static_assert(sizeof(RefinedContainer<std::vector<int>, SizeInterval<1>{}>) ==
              sizeof(std::vector<int>));

} // namespace refinery

#endif // REFINERY_REFINED_CONTAINER_HPP

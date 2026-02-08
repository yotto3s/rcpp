# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Test

```bash
# Configure with GCC build-tree xg++ (uninstalled)
cmake -B build --toolchain cmake/xg++-toolchain.cmake \
      -DGCC_BUILD_DIR=/path/to/gcc/build/gcc

# Or with an installed GCC 16+
cmake -B build -DCMAKE_CXX_COMPILER=g++-16

# Build and test
cmake --build build
ctest --test-dir build --output-on-failure

# Direct test execution
./build/tests/test_refine

# Build and run assembly comparison examples
cmake -B build ... -DREFINERY_BUILD_EXAMPLES=ON
cmake --build build --target asm-compare           # zero-overhead (compile-time paths)
cmake --build build --target asm-compare-runtime   # runtime-overhead (runtime_check + optional paths)
cmake --build build --target container-examples    # container showcase examples
```

Requires GCC 16+ with C++26 reflection support. The `-freflection` flag is added automatically by CMake for GCC. The `xg++-toolchain.cmake` file handles `-B`, `-nostdinc++`, libstdc++ include/link paths for uninstalled GCC build trees.

## Architecture

Header-only C++26 library providing Liquid Haskell-style refinement types. Everything lives in the `refinery::` namespace.

All public headers live in `include/refinery/`.

### Header Dependency Graph

```
refinery.hpp  (main entry point, type aliases)
  ├── diagnostics.hpp   (refinement_error, tag types, reflection-based formatting)
  ├── predicates.hpp    (35+ standard predicates: Positive, NonZero, InRange, Even, Finite, NotNaN, etc.)
  ├── compose.hpp       (All<>, Any<>, Not<>, If<>, runtime composition)
  ├── refined_type.hpp  (core Refined<T, Predicate> template)
  ├── operations.hpp    (safe arithmetic with predicate preservation traits, safe float math)
  ├── interval.hpp      (Interval<Lo,Hi> structural predicate, compile-time interval arithmetic)
  └── refined_container.hpp (RefinedContainer<C, SizePredicate>, size-predicated container wrappers)
domain.hpp    (domain-specific template aliases: Percentage<T>, Probability<T>, etc.)
  └── refinery.hpp
```

### Core Design

**`Refined<T, Predicate>`** wraps a value of type `T` and guarantees `Predicate(value)` holds. Three construction modes:
- `consteval` (default): compile-time verified, uses `std::meta::exception` on failure
- `runtime_check` tag: throws `refinement_error` on failure
- `assume_valid` tag: unchecked, for trusted contexts

**Predicates** are constexpr callable objects (lambdas) passed as non-type template parameters. Curried predicates (e.g., `InRange(lo, hi)`, `GreaterThan(n)`) return new predicates.

**Operations** (`operations.hpp`) use a `traits::preserves<Predicate, Operation>` trait system to determine if arithmetic preserves a refinement (e.g., Positive + Positive = Positive). When preservation is provable, operations return `Refined`; otherwise they return `std::optional<Refined>`. Safe float math functions (`safe_sqrt`, `safe_log`, `safe_asin`, `safe_acos`, `safe_reciprocal`) accept specifically-refined inputs (e.g., `Refined<T, Positive>`) and return `Refined` when the predicate is preserved, or plain `T` otherwise.

**Interval arithmetic** (`interval.hpp`) provides `Interval<Lo, Hi>` structural predicates that carry their bounds as template parameters. Arithmetic on interval-refined values computes result bounds at compile time (e.g., `[0,100] + [0,100]` yields `[0,200]`). The `IntervalRefined<T, Lo, Hi>` alias is a convenience for `Refined<T, Interval<Lo, Hi>{}>`.

**Container refinements** (`refined_container.hpp`) apply refinement types to container sizes. `RefinedContainer<Container, SizePredicate>` wraps a container and guarantees `SizePredicate(container.size())` holds, using the same three construction modes as `Refined<T, Predicate>`. `SizeInterval<Lo, Hi>` is a structural predicate analogous to `Interval<Lo, Hi>` for numeric values. Predicate-gated access (`front()`/`back()`) is only available when `lo >= 1` is provable at compile time. Move-based mutations (`push_back`, `pop_back`, `emplace_back`, `append`) consume `*this` and return a new `RefinedContainer` with shifted size bounds. The `freeze()` mechanism produces a `FrozenContainer` + `SizeGuard` pair for runtime-checked indexing with compile-time brand safety.

### Key Type Aliases (defined in `refinery.hpp`)

`PositiveI32`, `NonZeroI32`, `NonNegativeI32`, `PositiveI64`, `NonZeroI64`, `NonZeroU32`, `NonZeroU64`, `NonZeroUsize`, `PositiveF64`, `NonZeroF64`, `FiniteF32`, `FiniteF64`, `NormalizedF32`, `NormalizedF64`, etc. Domain template aliases (`Percentage<T>`, `Probability<T>`, `UnitFloat<T>`, `UnitDouble<T>`, `ByteValue<T>`, `PortNumber<T>`) with sensible defaults, plus concrete aliases (`Natural`, `Whole`) are in `domain.hpp`. Container type aliases: `SizeRefined<C, Lo, Hi>` (convenience for `RefinedContainer<C, SizeInterval<Lo, Hi>{}>`), `NonEmptyContainer<C>` (size >= 1).

### Zero-Overhead Examples (`examples/zero_overhead/`)

8 example files with paired `refined_*` / `plain_*` `__attribute__((noinline))` functions that prove zero runtime overhead at `-O2`. Covers: value passthrough, addition, multiplication, safe_sqrt, parameter passing, comparison operators, safe_divide/safe_reciprocal, and multi-op chains. Built with `-DREFINERY_BUILD_EXAMPLES=ON`. The `asm-compare` CMake target runs `scripts/compare_asm.sh` which uses `objdump -d` to extract, normalize (strip NOPs, alignment padding, RIP-relative offsets, branch target labels), and diff each pair.

### Runtime-Overhead Examples (`examples/runtime_overhead/`)

5 example files proving that runtime-checked paths (`runtime_check` construction and `std::optional` fallible operations) produce identical assembly to hand-written equivalents. Covers: runtime_check with Positive/NonZero/InRange predicates, and optional-returning integer addition/subtraction. The `asm-compare-runtime` CMake target compares these pairs.

### Container Examples (`examples/containers/`)

6 example files demonstrating `RefinedContainer` features: construction modes, predicate-gated access, move-based mutations, static indexing with interval-refined indices, range insertion from `std::array` and other `RefinedContainer`s, and freeze/guard branded indexing. Built with `-DREFINERY_BUILD_EXAMPLES=ON`. The `container-examples` CMake target builds all 6.

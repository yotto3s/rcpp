# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Test

```bash
# Configure and build
mkdir build && cd build
cmake ..
cmake --build .

# Run tests
ctest                    # via CTest (60s timeout)
./tests/test_refine      # direct execution

# Single command from repo root
cmake --build build && cd build && ctest --output-on-failure; cd ..
```

Requires GCC 16+ with C++26 reflection support. The `-freflection` flag is added automatically by CMake for GCC.

## Architecture

Header-only C++26 library providing Liquid Haskell-style refinement types. Everything lives in the `refine::` namespace.

All public headers live in `include/rcpp/`.

### Header Dependency Graph

```
refine.hpp  (main entry point, type aliases, convenience macros)
  ├── diagnostics.hpp   (refinement_error, tag types, reflection-based formatting)
  ├── predicates.hpp    (30+ standard predicates: Positive, NonZero, InRange, Even, etc.)
  ├── compose.hpp       (All<>, Any<>, Not<>, If<>, runtime composition)
  ├── refined.hpp       (core Refined<T, Predicate> template)
  └── operations.hpp    (safe arithmetic with predicate preservation traits)
```

### Core Design

**`Refined<T, Predicate>`** wraps a value of type `T` and guarantees `Predicate(value)` holds. Three construction modes:
- `consteval` (default): compile-time verified, uses `std::meta::exception` on failure
- `runtime_check` tag: throws `refinement_error` on failure
- `assume_valid` tag: unchecked, for trusted contexts

**Predicates** are constexpr callable objects (lambdas) passed as non-type template parameters. Curried predicates (e.g., `InRange(lo, hi)`, `GreaterThan(n)`) return new predicates.

**Operations** (`operations.hpp`) use a `traits::preserves<Predicate, Operation>` trait system to determine if arithmetic preserves a refinement (e.g., Positive + Positive = Positive). When preservation is provable, operations return `Refined`; otherwise they return `std::optional<Refined>`.

### Key Type Aliases (defined in `refine.hpp`)

`PositiveInt`, `NonZeroInt`, `NonNegativeInt`, `Percentage`, `Probability`, `ByteValue`, `PortNumber`, `Natural`, `Whole`, etc. Created via `DEFINE_REFINED_TYPE` and `DEFINE_PREDICATE` macros.

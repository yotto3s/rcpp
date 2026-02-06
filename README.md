# refinery - C++26 Refinement Types Library

A header-only library providing Liquid Haskell-style refinement types for C++26, using GCC reflection for rich compile-time error messages.

## Features

- **Compile-time verification**: Values are verified at compile time using `consteval` constructors
- **Runtime verification**: Optional runtime checking with `runtime_check` tag
- **Reflection-powered diagnostics**: Compile-time error messages include the actual violating value via C++26 reflection
- **Standard predicates**: `Positive`, `NonZero`, `NonNegative`, `InRange`, `Even`, `Odd`, etc.
- **Float predicates**: `Finite`, `NotNaN`, `IsNaN`, `IsInf`, `IsNormal`, `ApproxEqual`
- **Predicate composition**: `All<P1,P2>`, `Any<P1,P2>`, `Not<P>`, `If<P1,P2>`, etc.
- **Interval arithmetic**: `Interval<Lo, Hi>` structural predicates with compile-time arithmetic — `[1,10] + [1,10]` automatically yields `[2,20]`
- **Type-safe operations**: `safe_divide`, `safe_sqrt`, `safe_log`, `safe_asin`, `safe_acos`, `safe_reciprocal`, `abs`, `square`, `refined_min/max`
- **Float type aliases**: `FiniteF64`, `NormalizedF64`, etc.
- **Domain aliases**: `Percentage<>`, `Probability<>`, `UnitDouble<>`, `PortNumber<>`, etc. — type-parameterized with sensible defaults (`#include <refinery/domain.hpp>`)
- **Zero runtime overhead**: Assembly-verified — `Refined<T>` produces identical machine code to raw `T` at `-O2`

## Requirements

- GCC 16+ built from source with `-freflection` support (C++26 reflection)
- CMake 3.20+

## Building with a GCC build tree

If you built GCC from source and have an uninstalled build tree, use the provided toolchain file:

```bash
cmake -B build --toolchain cmake/xg++-toolchain.cmake \
      -DGCC_BUILD_DIR=/path/to/gcc/build/gcc
cmake --build build
ctest --test-dir build
```

`GCC_BUILD_DIR` should point to the `gcc/` subdirectory of your GCC build tree (the directory containing `xg++` and `cc1plus`). The toolchain file automatically handles:
- Setting `xg++` as the compiler with the `-B` flag for `cc1plus` lookup
- Adding `-nostdinc++` and the correct `-isystem` paths for the build-tree libstdc++
- Linker paths and rpath for the build-tree libstdc++

## Usage

```cpp
#include <refinery/refinery.hpp>
using namespace refinery;

// Compile-time verified
constexpr PositiveI32 x{42};      // OK
// PositiveI32 y{-1};             // Compile error: "Refinement violation: -1 does not satisfy predicate"

// Runtime checked
auto z = try_refine<PositiveI32>(user_input);
if (z) use(*z);

// Type-safe division
template<typename T>
T safe_divide(T num, Refined<T, NonZero> denom) {
    return num / denom.get();  // Can never divide by zero!
}

// Safe math on floats
PositiveF64 pd{9.0, runtime_check};
auto root = safe_sqrt(pd);   // Returns Refined<double, Positive>
double lg = safe_log(pd);    // Returns double (log can be negative)
```

### Compile-Time Error Messages

When a predicate fails at compile time, GCC's reflection produces a clear diagnostic:

```
error: 'consteval' call is not a constant expression
note: explicitly thrown exception with message
      "Refinement violation: -1 does not satisfy predicate"
```

## Interval Arithmetic

`Interval<Lo, Hi>` is a structural predicate representing a closed range `[Lo, Hi]`. Arithmetic on interval-refined values computes the result bounds at compile time:

```cpp
using Score = IntervalRefined<int, 0, 100>;
Score a{30, runtime_check};
Score b{40, runtime_check};

auto sum = a + b;  // type: Refined<int, Interval<0, 200>>
auto neg = -a;     // type: Refined<int, Interval<-100, 0>>
```

Supported operations: addition, subtraction, multiplication, unary negation. All bound computation happens at compile time with zero runtime cost.

## Factory & Utility Functions

| Function | Returns | On failure |
|----------|---------|------------|
| `Refined<T,P>(val)` | `Refined<T,P>` | Compile error (consteval) |
| `Refined<T,P>(val, runtime_check)` | `Refined<T,P>` | Throws `refinement_error` |
| `Refined<T,P>(val, assume_valid)` | `Refined<T,P>` | UB if predicate fails |
| `try_refine<RefinedT>(val)` | `optional<RefinedT>` | Returns `nullopt` |
| `make_refined<Pred>(val)` | `Refined<T,Pred>` | Compile error (consteval) |
| `make_refined_checked<Pred>(val)` | `Refined<T,Pred>` | Throws `refinement_error` |
| `assume_refined<Pred>(val)` | `Refined<T,Pred>` | UB if predicate fails |
| `refine_to<To>(from)` | `To` | Throws `refinement_error` |
| `try_refine_to<To>(from)` | `optional<To>` | Returns `nullopt` |
| `transform_refined<Pred>(refined, fn)` | `Refined<R,Pred>` | Throws `refinement_error` |

## Zero-Overhead Verification

The `examples/zero_overhead/` directory contains 8 paired benchmarks proving `Refined<T>` compiles to the same instructions as raw `T`. Each file has `refined_*` and `plain_*` function pairs; the `asm-compare` target disassembles the binaries and diffs the normalized assembly.

```bash
cmake -B build --toolchain cmake/xg++-toolchain.cmake \
      -DGCC_BUILD_DIR=/path/to/gcc/build/gcc \
      -DREFINERY_BUILD_EXAMPLES=ON
cmake --build build --target asm-compare
```

All 10 comparisons pass (identical instructions):

| Example | What it proves |
|---------|---------------|
| `01_value_passthrough` | `.get()` accessor is free |
| `02_arithmetic` | `Positive + Positive` == `int + int` |
| `03_safe_sqrt` | `safe_sqrt(NonNegative)` == `std::sqrt()` |
| `04_parameter_passing` | `Refined<int>` passes in registers like `int` |
| `05_comparison` | `operator<` / `operator==` == plain comparison |
| `06_multiply` | `Positive * Positive` == `int * int` |
| `07_safe_divide` | `safe_divide` / `safe_reciprocal` == plain division |
| `08_chain` | Multi-op chain == plain math equivalent |

## Building

With an installed GCC 16+:

```bash
cmake -B build -DCMAKE_CXX_COMPILER=g++-16
cmake --build build
ctest --test-dir build
```

With an uninstalled GCC build tree (see above):

```bash
cmake -B build --toolchain cmake/xg++-toolchain.cmake \
      -DGCC_BUILD_DIR=/path/to/gcc/build/gcc
cmake --build build
ctest --test-dir build
```

## Installation

```bash
cmake --install . --prefix /usr/local
```

Then in your project:

```cmake
find_package(refinery REQUIRED)
target_link_libraries(your_target PRIVATE refinery::refinery)
```

## License

MIT License

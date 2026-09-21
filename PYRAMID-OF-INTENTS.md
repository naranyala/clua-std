# Pyramid of Intents

This document defines what `clua-std` is trying to become and how decisions
should be prioritized. Every item in [TODOS.md](TODOS.md) must reference at
least one intent from this document.

## North-star intent

Make common low-level data operations available to Lua through a small,
predictable, safe, and portable native standard library.

The project should feel like a natural Lua module while providing the
performance and memory representation of C where that matters.

## Product promise

`require("cl")` should provide dependable primitives for:

1. Mutable byte storage.
2. Binary integer encoding and decoding.
3. Checksums and other low-level data utilities.
4. Small numeric and bitwise operations that benefit from native code.

Each primitive should have explicit indexing, range, error, and ownership
semantics. A caller should not need to use raw pointers or write C to handle
ordinary binary data safely.

## Intent hierarchy

### I1 — Be safe by default

Protect Lua programs from out-of-bounds access, invalid integer ranges,
undefined shifts, accidental ownership confusion, and unchecked allocation
failures.

Success means invalid input produces a clear Lua error and valid input cannot
cause the library to read or write outside its intended memory region.

### I2 — Make the API small and coherent

Expose a compact `cl` namespace with regular naming, zero-based byte offsets,
consistent return values, and composable operations. Avoid adding features
that duplicate the Lua standard library or create multiple incompatible ways
to do the same thing.

Success means a user can learn the API from the README and predict the name,
arguments, and result of a related operation.

### I3 — Provide useful binary-data primitives

Make buffers, fixed-width integers, endianness, and checksums reliable enough
for packet formats, file formats, embedded protocols, and test fixtures.

Success means binary values round-trip exactly, offsets work for both strings
and buffers, and behavior is independent of host endianness.

### I4 — Keep Lua ergonomics over a native core

Keep application flow and convenient composition in Lua, while placing
memory-sensitive storage and computational kernels in C. Keep the
Lua-independent algorithms testable without requiring a Lua runtime where
practical.

Success means the boundary is intentional: conversions and validation live at
the binding boundary, and algorithms do not depend on Lua state unnecessarily.

### I5 — Be portable and easy to build

Support Lua 5.3+ with a 64-bit Lua integer type across common Unix-like
platforms, and make compiler, linker, and Lua-library assumptions explicit.

Success means a new contributor can configure, build, test, and run the
example using documented commands, with failures diagnosed clearly.

### I6 — Be verifiable and maintainable

Treat the public API, edge cases, ABI assumptions, and performance-sensitive
behavior as testable contracts. Keep documentation, tests, and implementation
in agreement.

Success means changes come with focused tests and the project can distinguish
functional regressions from environment or toolchain failures.

## Decision order

When intents conflict, use this order:

1. Safety and memory correctness (`I1`).
2. Public API consistency (`I2`).
3. Correct binary behavior (`I3`).
4. Clear Lua/C separation (`I4`).
5. Portability and buildability (`I5`).
6. Optimization and additional convenience (`I6`, only when they do not
   weaken the higher levels).

## Non-goals

The project is not currently intended to be:

- a general-purpose replacement for Lua's standard library;
- a serialization framework with schema discovery or reflection;
- a garbage-collected object system exposed wholesale from C;
- a platform-specific systems API;
- a benchmark-driven collection of obscure micro-optimizations.

New scope should be admitted only when it strengthens the north-star intent
and has a clear place in this hierarchy.

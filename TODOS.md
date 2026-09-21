# TODOs

This backlog is intentionally intent-driven. A TODO is actionable only when
it has an intent reference, a concrete completion condition, and a validation
method. IDs are stable references for issues, commits, and reviews.

## P0 — Make the project build and test reliably

- [x] **T-001 — Fix the strict-build failure** · Intent: `I5`, `I6`
  - Determine why Lua 5.4's `luaL_newlib` macro triggers `-Woverflow` under
    the supported compiler and replace or wrap the affected registration
    path without weakening all warnings.
  - Done when the default `cmake -S . -B build && cmake --build build`
    succeeds with `-Werror`.
  - Validate with a clean configure and build on the local Lua installation.

- [ ] **T-002 — Define the supported Lua linking model** · Intent: `I5`
  - Document and/or detect the requirement for a shared or PIC-compatible
    Lua library when producing the loadable `cl` module.
  - Prefer a portable CMake configuration that works with shared Lua and
    gives a direct diagnostic for non-PIC static Lua.
  - Done when the documented environment can link `cl.so` successfully.

- [ ] **T-003 — Make CTest execute the built module** · Intent: `I5`, `I6`
  - Verify the test environment handles the platform's module suffix instead
    of assuming only `.so`.
  - Done when `ctest --test-dir build --output-on-failure` actually loads the
    freshly built module and passes.

## P1 — Lock down safety and behavioral contracts

- [ ] **T-010 — Expand invalid-input tests** · Intent: `I1`, `I6`
  - Cover negative sizes and offsets, empty buffers, zero-length ranges,
    overflow-sized requests, invalid byte values, malformed numeric arrays,
    and wrong Lua types.
  - Done when each public operation has at least one boundary and one type
    failure test.

- [ ] **T-011 — Test integer and bitwise edge cases** · Intent: `I1`, `I3`, `I6`
  - Cover signed minima/maxima, unsigned maxima, all supported widths and
    endiannesses, integer-width shift limits, zero/full-width rotations, and
    negative rotation counts.
  - Done when results and error behavior are specified by tests rather than
    only by implementation details.

- [ ] **T-012 — Audit allocation and Lua error paths** · Intent: `I1`, `I4`
  - Review every allocation, temporary buffer, and Lua API call for cleanup
    and safe failure behavior, especially in `stats` input conversion.
  - Done when the audit is recorded in code comments or documentation and no
    identified path leaks memory or leaves partially initialized userdata.

## P1 — Make the public API coherent and documented

- [x] **T-020 — Publish a complete API reference** · Intent: `I2`, `I6`
  - Document signatures, defaults, zero-based indexing, return values,
    mutability, accepted source types, and error conditions for every public
    function.
  - Done when the README covers all exported functions or links to a complete
    reference without requiring source inspection.

- [x] **T-021 — Specify buffer ownership and copying semantics** · Intent: `I1`, `I2`, `I4`
  - Explicitly describe userdata lifetime, `from_string` copying, `slice`
    independence, overlapping `copy`, and whether buffers may be retained by
    C calls.
  - Done when tests demonstrate the documented semantics.

- [ ] **T-022 — Define numeric semantics for `stats`** · Intent: `I2`, `I6`
  - Document handling of NaN, infinities, integer coercion, summation order,
    and empty input. The current contract rejects non-finite values and empty
    input, and preserves input order for summation.
  - Add runtime coverage once the native module can be linked in the test
    environment.

## P2 — Improve portability and contributor workflow

- [ ] **T-030 — Add a portable module-suffix configuration** · Intent: `I5`
  - Configure CMake and CTest for `.so`, `.dylib`, and `.dll`/Lua module
    naming conventions where applicable.
  - Done when the project no longer relies on a Linux-only `LUA_CPATH` value.

- [ ] **T-031 — Add CI coverage for supported combinations** · Intent: `I5`, `I6`
  - Test at least one Lua 5.3+ configuration and the primary supported
    compiler/platform combinations.
  - Include a strict-warning build and the Lua test suite.
  - A GitHub Actions workflow for Ubuntu/Lua 5.4 is now present; remote CI
    validation remains pending.

- [ ] **T-032 — Separate algorithm tests from Lua binding tests** · Intent: `I4`, `I6`
  - Add C-level tests for CRC32, endian helpers, and statistics, while
    retaining Lua tests for conversions, userdata, and public API behavior.
  - Done when failures identify whether the core algorithm or binding is at
    fault. Core tests and a separate CTest target are now implemented; runtime
    validation remains pending until the native module can link locally.

## P3 — Add only high-value capabilities

- [x] **T-040 — Establish an extension policy before adding APIs** · Intent: `I2`, `I3`, `I6`
  - Define criteria for adding another binary width, checksum, buffer method,
    or numeric primitive: clear use case, naming fit, safety contract, tests,
    and documentation.
  - Done when future feature proposals can be evaluated without expanding
    scope by default.

- [ ] **T-041 — Benchmark representative workloads** · Intent: `I4`, `I6`
  - Measure buffer operations, integer packing/reading, CRC32, and statistics
    against idiomatic Lua baselines.
  - Use results to guide optimization only where it improves meaningful
    workloads without harming portability or clarity.

## Backlog rules

1. No TODO may be added without an `I#` intent reference.
2. A TODO is complete only when its stated validation passes.
3. New features remain below build, safety, API, and portability work unless
   they remove a demonstrated blocker.
4. When an intent changes, update this file and re-check every linked TODO.

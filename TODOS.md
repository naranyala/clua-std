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

## P2 — Plan the next primitive families before implementing them

These items are deliberately written as contracts and sequencing work. No
new primitive should be implemented from this section until its contract,
tests, documentation, and example plan are agreed in the corresponding TODO.

- [ ] **T-050 — Create the primitive inventory and naming matrix** · Intent: `I2`, `I3`, `I6`
  - Enumerate candidate operations by family: bytes, buffers, binary codecs,
    flags, checksums, text/encoding, cursors, and numeric reductions.
  - For every candidate record the namespace, signature, accepted source
    types, return type, indexing rules, error behavior, and likely use case.
  - Mark each candidate as `commit`, `defer`, or `reject` before coding.
  - Done when every proposed primitive has a decision and no implementation
    begins from an untracked idea.

- [ ] **T-051 — Stabilize the human-friendly byte facade** · Intent: `I1`, `I2`, `I3`
  - Treat `cl.bytes.to_hex`, `from_hex`, and `concat` as the first ergonomic
    facade while keeping `cl.mem` and `cl.bin` as precise low-level APIs.
  - Decide whether future text encodings belong under `cl.bytes`, `cl.text`,
    or a separate namespace; do not add aliases without a compatibility need.
  - Define empty-input, invalid-character, allocation, and result-ownership
    semantics.
  - Done when the facade has documentation, boundary tests, examples, and a
    stable compatibility statement.
  - Implementation is present for hex conversion and concatenation; the
    compatibility statement and runtime validation remain open.

- [ ] **T-052 — Stabilize the human-friendly flags facade** · Intent: `I1`, `I2`, `I3`
  - Define `cl.flags.has`, `set`, and `clear` around zero-based bit indexes,
    including behavior at the platform integer-width boundary.
  - Decide whether named flag sets are data tables in Lua or a future native
    abstraction; prefer Lua data tables until a real performance need exists.
  - Done when `cl.flags` and `cl.bits` have clearly separated ergonomic and
    primitive responsibilities, with no duplicate undocumented behavior.
  - Implementation and example usage are present; the final compatibility
    decision remains open.

- [ ] **T-053 — Decide the binary-number roadmap** · Intent: `I2`, `I3`, `I5`
  - Evaluate 24-bit integers, checked 64-bit integers, varints, zigzag signed
    integers, and floating-point values as separate proposals.
  - For 64-bit unsigned values, explicitly choose between rejection, a byte
    string result, or a multi-word representation because a signed 64-bit Lua
    integer cannot represent the full unsigned range exactly.
  - For every accepted format define wire layout, range, precision, endian
    behavior, and host portability requirements.
  - Done when the supported binary matrix is documented and rejected formats
    have written reasons rather than silently becoming future scope.

- [ ] **T-054 — Design a cursor/reader abstraction** · Intent: `I1`, `I2`, `I3`, `I4`
  - Evaluate a sequential reader for strings and buffers with operations such
    as `read_u16le`, `read_f32be`, `remaining`, and `position`.
  - Define whether it is immutable, whether reads advance state on failure,
    and whether it returns values, slices, or errors.
  - Compare a Lua table implementation against C userdata before choosing an
    implementation; the abstraction must not duplicate every existing reader
    without reducing protocol boilerplate.
  - Done when one packet-lab example demonstrates a measurable readability
    improvement and all cursor boundary rules are tested.

- [ ] **T-055 — Design a safe buffer algorithm family** · Intent: `I1`, `I2`, `I3`
  - Evaluate `find`, `starts_with`, `ends_with`, `reverse`, and constant-time
    equality as separate candidates for `cl.mem` or `cl.bytes`.
  - Define whether needles may be strings, buffers, or both, and define empty
    needle behavior before implementation.
  - Reject convenience methods that only wrap one trivial Lua expression
    unless they remove a demonstrated allocation or safety hazard.
  - Done when accepted methods have complexity, ownership, and overlap
    semantics documented and tested.
  - `find`, `starts_with`, `ends_with`, and `reverse` are implemented with
    tests and API documentation; runtime validation remains open.

- [ ] **T-056 — Select the checksum and digest roadmap** · Intent: `I3`, `I4`, `I6`
  - Evaluate Adler-32, CRC-16 variants, FNV-1a, and cryptographic hashes as
    distinct scope decisions rather than one generic `hash` namespace.
  - Keep cryptographic functionality out of scope unless a maintained,
    portable dependency and a concrete user need are identified.
  - Done when the project documents which checksum families it supports,
    their parameters, and known-value test vectors.

- [ ] **T-057 — Design numeric reduction extensions** · Intent: `I2`, `I4`, `I6`
  - Evaluate median, variance, standard deviation, dot products, and min/max
    reductions based on real example workloads.
  - Define empty-input, non-finite, precision, and summation semantics before
    adding functions beside `cl.math.stats`.
  - Prefer one coherent result structure over a large list of aliases.
  - Done when at least one integration example demonstrates the need and the
    numerical contract has reference tests.

## P2 — Implement each accepted primitive through integration examples

- [ ] **T-060 — Create a primitive-to-example coverage matrix** · Intent: `I2`, `I3`, `I6`
  - Map every public primitive to at least one example and every example to a
    user-facing scenario: packet parsing, file records, telemetry, flags,
    checksums, or numeric analysis.
  - Add a check that new public functions are either demonstrated or explicitly
    documented as low-level-only.
  - Done when `examples/packet-lab` and future examples show complete coverage
    without duplicating the test suite.
  - A first matrix is documented in `docs/primitive-coverage.md`; automated
    enforcement remains open.

- [ ] **T-061 — Expand packet-lab into a protocol cookbook** · Intent: `I2`, `I3`, `I4`, `I6`
  - Add examples for malformed packets, optional fields, version negotiation,
    floating-point measurements, flag evolution, and checksum rejection.
  - Keep each example runnable independently and include expected output or
    an assertion describing the behavior.
  - Done when the project demonstrates both low-level primitives and the
    human-friendly facade in one coherent subproject.
  - The malformed-packet validation example is now present; optional fields
    and version negotiation remain future examples.

- [ ] **T-062 — Add a second domain example** · Intent: `I3`, `I4`, `I6`
  - Build a small file-record or log-index example that exercises a different
    workflow from telemetry packets, such as offsets, record boundaries, and
    random access.
  - Use this to expose APIs that are useful outside network protocols and to
    prevent packet-lab from defining the entire library design.
  - Done when the example has its own README, fixtures, validation, and a
    clear reason to use `clua-std`.
  - `examples/record-index` now covers indexed binary records; automated
    execution and fixture expansion remain open.

## P1 — Release gates for the expanded standard library

- [ ] **T-070 — Establish API compatibility rules** · Intent: `I2`, `I5`, `I6`
  - Define what counts as a breaking change to names, argument order, return
    types, errors, byte order, ownership, and version strings.
  - Require a version change and migration note for breaking behavior.
  - Done when the rules are documented and the current `0.5.0` surface is
    labeled experimental, stable, or compatibility-frozen.
  - The policy is documented in `docs/compatibility.md`; the current label is
    Experimental until runtime/portability gates pass.

- [ ] **T-071 — Add generated API surface verification** · Intent: `I2`, `I6`
  - Maintain one authoritative list of exported namespaces and functions and
    compare it with the documentation and registration tables.
  - Detect accidental exports, stale docs, and version mismatches in CI.
  - Done when an API change cannot merge without updating its contract and
    at least one test.

- [ ] **T-072 — Define performance budgets for native primitives** · Intent: `I4`, `I6`
  - Benchmark representative buffer, binary, checksum, flag, and reduction
    operations against Lua baselines.
  - Set budgets only after measuring realistic workloads; do not optimize
    convenience functions solely because they are implemented in C.
  - Done when benchmark results inform which candidates from T-050 are worth
    native implementation.
  - A repeatable local harness is present in `benchmarks/`; measured budgets
    and baseline results remain open.

## P2 — Close the remaining usability and release gaps

- [ ] **T-080 — Make end-to-end Lua validation executable in supported environments** · Intent: `I5`, `I6`
  - Provide a documented shared/PIC Lua setup and make CTest fail with an
    actionable diagnostic when the module cannot be linked.
  - Run the complete Lua test suite and every integration example in CI.
  - Done when a supported environment proves `require("cl")`, CTest, and the
    examples all work from a clean checkout.

- [ ] **T-081 — Add installation and release packaging** · Intent: `I5`, `I6`
  - Add a CMake install target for the module, public documentation, and
    example metadata without installing build artifacts.
  - Define package layout, release archive contents, and version sourcing.
  - Done when `cmake --install` produces a usable module tree and a clean
    release archive can be generated reproducibly.

- [ ] **T-082 — Add property and fuzz testing for byte boundaries** · Intent: `I1`, `I3`, `I6`
  - Generate random buffers and values for pack/read round-trips, slicing,
    copying, hex conversion, and malformed source lengths.
  - Keep fuzz tests bounded and deterministic in ordinary CI, with an optional
    sanitizer/fuzzer job for longer runs.
  - Done when failures identify an input seed and the suite covers all public
    byte-source primitives.

- [ ] **T-083 — Verify every integration example automatically** · Intent: `I4`, `I5`, `I6`
  - Add a CMake/CTest or CI runner that executes each example from its intended
    working directory with the freshly built module path.
  - Capture expected output only where output is part of the contract; use
    assertions for behavior.
  - Done when adding a new example without a runnable check is impossible or
    visibly reported by CI.

- [ ] **T-084 — Generate and verify the public API surface** · Intent: `I2`, `I6`
  - Maintain one machine-readable namespace/function inventory and compare it
    with C registration tables, API documentation, and Lua tests.
  - Detect stale version strings and undocumented exports.
  - Done when CI reports missing, extra, or undocumented public functions.

- [ ] **T-085 — Establish a release checklist and support statement** · Intent: `I2`, `I5`, `I6`
  - Define supported Lua versions, integer/ABI assumptions, operating systems,
    compilers, linker requirements, test evidence, and known limitations.
  - Require the checklist for every versioned release.
  - Done when users can distinguish experimental, supported, and unsupported
    environments without reading implementation source.

## Backlog rules

1. No TODO may be added without an `I#` intent reference.
2. A TODO is complete only when its stated validation passes.
3. New features remain below build, safety, API, and portability work unless
   they remove a demonstrated blocker.
4. When an intent changes, update this file and re-check every linked TODO.
5. Primitive work follows this order: inventory, contract, core algorithm,
   binding, tests, documentation, example, then benchmark.
## Recommended development roadmap

This roadmap captures the broader development strategy. The actionable
contracts remain the stable T-IDs above; completed work must update both the
contract and its validation evidence.

### Phase 1 — Make the current core dependable

- [ ] Expand tests for invalid offsets, oversized lengths, integer overflow,
  empty buffers, maximum supported sizes, allocation failures, and partial
  operations.
- [ ] Add AddressSanitizer, UndefinedBehaviorSanitizer, and optional
  Valgrind/MemorySanitizer CI jobs.
- [ ] Measure coverage of `csrc/core.c`, buffer ownership paths, and all
  public Lua bindings.
- [ ] Fuzz binary decoders, buffer operations, CRC-32, and hexadecimal
  conversion.
- [ ] Define behavior for allocation failure, very large buffers, integer
  overflow, and partial operations.
- [ ] Benchmark buffer copying, slicing, binary decoding, and large-buffer
  operations.

### Phase 2 — Improve the developer experience

- [ ] Add a CMake install/export package with a stable imported target.
- [ ] Add a stable module version and compatibility policy.
- [ ] Generate API documentation from one authoritative specification.
- [ ] Expand API documentation with argument types, return values, mutation
  behavior, ownership, and error conditions.
- [ ] Add optional Lua type definitions for editor support.
- [ ] Add a compatibility test suite for multiple Lua versions.

### Phase 3 — Strengthen the architecture

- [ ] Extract repeated validation into small reusable helpers.
- [ ] Split codecs, buffers, statistics, and flags into internal modules if
  `csrc/cl.c` becomes difficult to navigate.
- [ ] Keep the public Lua API stable while allowing internal C refactors.
- [ ] Standardize error categories and messages at the Lua boundary.
- [ ] Apply integer-safe size calculations before allocations and pointer
  arithmetic.
- [ ] Mark public APIs as stable, experimental, or deprecated.

### Phase 4 — Add high-value capabilities

- [ ] Add safe file and stream integration with explicit allocation limits,
  partial I/O handling, and record codecs.
- [ ] Add encoding utilities such as Base16/Base64/Base32, UTF-8 validation,
  and constant-time comparison.
- [ ] Add binary readers/writers, growable builders, concatenated buffers,
  and explicitly owned zero-copy views where justified.
- [ ] Add checked integer conversion, saturating arithmetic, and explicit-width
  bit helpers.
- [ ] Add interoperability with Lua file descriptors, callbacks, and safe
  buffer/string conversion.

### Phase 5 — Distribution and ecosystem

- [ ] Test Linux, macOS, and Windows in CI.
- [ ] Support Lua 5.3, Lua 5.4, and LuaJIT when technically compatible.
- [ ] Publish reproducible release artifacts and semantic-version releases.
- [ ] Maintain a changelog, migration guide, and compatibility matrix.
- [ ] Provide LuaRocks or equivalent distribution packaging.

### Immediate implementation order

1. Add sanitizer and coverage CI.
2. Add comprehensive failure-path and boundary tests.
3. Add fuzz targets for binary parsers and buffer operations.
4. Introduce explicit allocation and size limits.
5. Add CMake installation/export support.
6. Add a binary reader/writer abstraction.
7. Add safe file I/O helpers.
8. Improve documentation and versioning.
9. Expand portability testing.

The guiding principle is to preserve the safety model, make contracts explicit,
and add features only after correctness infrastructure can detect regressions.
